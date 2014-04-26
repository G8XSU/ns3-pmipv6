/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jaume Nin <jnin@cttc.cat>
 *         Nicola Baldo <nbaldo@cttc.cat>
 */


#include "epc6-sgw-application.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv6.h"
#include "ns3/inet-socket-address.h"
#include "ns3/epc-gtpu-header.h"
#include "ns3/abort.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/simulator.h"
#include "ns3/config.h"

#include "ns3/icmpv6-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Epc6SgwApplication");

NS_OBJECT_ENSURE_REGISTERED (Epc6SgwApplication);

/////////////////////////
// UeInfo
/////////////////////////

Epc6SgwApplication::UeInfo::UeInfo ()
{
  NS_LOG_FUNCTION (this);
}

void
Epc6SgwApplication::UeInfo::AddBearer (Ptr<EpcTft> tft, uint8_t bearerId, uint32_t teid)
{
  NS_LOG_FUNCTION (this << tft << teid);
  m_teidByBearerIdMap[bearerId] = teid;
  return m_tftClassifier.Add (tft, teid);
}

uint32_t
Epc6SgwApplication::UeInfo::Classify (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  // we hardcode DOWNLINK direction since the PGW is expected to
  // classify only downlink packets (uplink packets will go to the
  // internet without any classification). 
  return m_tftClassifier.Classify (p, EpcTft::DOWNLINK);
}

Ipv6Address
Epc6SgwApplication::UeInfo::GetEnbAddr ()
{
  return m_enbAddr;
}

void
Epc6SgwApplication::UeInfo::SetEnbAddr (Ipv6Address enbAddr)
{
  m_enbAddr = enbAddr;
}

Ipv6Address
Epc6SgwApplication::UeInfo::GetUePrefix ()
{
  return m_uePrefix;
}

void
Epc6SgwApplication::UeInfo::SetUePrefix (Ipv6Address uePrefix)
{
  m_uePrefix = uePrefix;
}


TypeId
Epc6SgwApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Epc6SgwApplication")
    .SetParent<Object> ()
    ;
  return tid;
}

void
Epc6SgwApplication::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_s1uSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_s1uSocket = 0;
  delete (m_s11SapSgw);
}

Epc6SgwApplication::Epc6SgwApplication (const Ptr<VirtualNetDevice> tunDevice, const Ptr<Socket> s1uSocket)
  : m_s1uSocket (s1uSocket),
    m_tunDevice (tunDevice),
    m_gtpuUdpPort (2152), // fixed by the standard
    m_teidCount (0),
    m_s11SapMme (0)
{
  NS_LOG_FUNCTION (this << tunDevice << s1uSocket);
  m_s1uSocket->SetRecvCallback (MakeCallback (&Epc6SgwApplication::RecvFromS1uSocket, this));
  m_s11SapSgw = new MemberEpcS11SapSgw<Epc6SgwApplication> (this);
}

Epc6SgwApplication::~Epc6SgwApplication ()
{
  NS_LOG_FUNCTION (this);
}

bool
Epc6SgwApplication::RecvFromTunDevice (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << source << dest << packet << packet->GetSize () << protocolNumber);

  // get IP address of UE
  Ptr<Packet> pCopy = packet->Copy ();
  Ipv6Header ipv6Header;
  pCopy->RemoveHeader (ipv6Header);
  Ipv6Address ueAddr =  ipv6Header.GetDestinationAddress ();
  NS_LOG_LOGIC ("packet addressed to UE " << ueAddr);

  // Derive prefix from UE prefix from the address.
  uint8_t buf[16];
  ueAddr.GetBytes (buf);
  for (int i = 8; i < 16; i++)
    buf[i] = 0;
  Ipv6Address uePrefix (buf);
  // find corresponding UeInfo address
  std::map<Ipv6Address, Ptr<UeInfo> >::iterator it = m_ueInfoByPrefixMap.find (uePrefix);
  if (it == m_ueInfoByPrefixMap.end ())
    {        
      NS_LOG_WARN ("unknown UE address " << ueAddr) ;
    }
  else
    {
      Ipv6Address enbAddr = it->second->GetEnbAddr ();
      uint32_t teid = it->second->Classify (packet);   
      if (teid == 0)
        {
          NS_LOG_WARN ("no matching bearer for this packet");                   
        }
      else
        {
          SendToS1uSocket (packet, enbAddr, teid);
        }
    }
  // there is no reason why we should notify the TUN
  // VirtualNetDevice that he failed to send the packet: if we receive
  // any bogus packet, it will just be silently discarded.
  const bool succeeded = true;
  return succeeded;
}

void 
Epc6SgwApplication::RecvFromS1uSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);  
  NS_ASSERT (socket == m_s1uSocket);
  Ptr<Packet> packet = socket->Recv ();
  GtpuHeader gtpu;
  packet->RemoveHeader (gtpu);
  uint32_t teid = gtpu.GetTeid ();

  /// \internal
  /// Workaround for \bugid{231}
  SocketAddressTag tag;
  packet->RemovePacketTag (tag);

  SendToTunDevice (packet, teid);
}

void 
Epc6SgwApplication::SendToTunDevice (Ptr<Packet> packet, uint32_t teid)
{
  NS_LOG_FUNCTION (this << packet << teid);
  NS_LOG_LOGIC (" packet size: " << packet->GetSize () << " bytes");
  m_tunDevice->Receive (packet, Ipv6L3Protocol::PROT_NUMBER, m_tunDevice->GetAddress (), m_tunDevice->GetAddress (), NetDevice::PACKET_HOST);
}

void 
Epc6SgwApplication::SendToS1uSocket (Ptr<Packet> packet, Ipv6Address enbAddr, uint32_t teid)
{
  NS_LOG_FUNCTION (this << packet << enbAddr << teid);

  GtpuHeader gtpu;
  gtpu.SetTeid (teid);
  // From 3GPP TS 29.281 v10.0.0 Section 5.1
  // Length of the payload + the non obligatory GTP-U header
  gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);  
  packet->AddHeader (gtpu);
  uint32_t flags = 0;
  m_s1uSocket->SendTo (packet, flags, Inet6SocketAddress(enbAddr, m_gtpuUdpPort));
}


void 
Epc6SgwApplication::SetS11SapMme (EpcS11SapMme * s)
{
  m_s11SapMme = s;
}

EpcS11SapSgw* 
Epc6SgwApplication::GetS11SapSgw ()
{
  return m_s11SapSgw;
}

void 
Epc6SgwApplication::AddEnb (uint16_t cellId, Ipv6Address enbAddr, Ipv6Address sgwAddr)
{
  NS_LOG_FUNCTION (this << cellId << enbAddr << sgwAddr);
  EnbInfo enbInfo;
  enbInfo.enbAddr = enbAddr;
  enbInfo.sgwAddr = sgwAddr;
  m_enbInfoByCellId[cellId] = enbInfo;
}

void 
Epc6SgwApplication::AddUe (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  Ptr<UeInfo> ueInfo = Create<UeInfo> ();
  m_ueInfoByImsiMap[imsi] = ueInfo;
}

void 
Epc6SgwApplication::SetUePrefix (uint64_t imsi, Ipv6Address uePrefix)
{
  NS_LOG_FUNCTION (this << imsi << uePrefix);
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi); 
  m_ueInfoByPrefixMap[uePrefix] = ueit->second;
  ueit->second->SetUePrefix (uePrefix);
}

void
Epc6SgwApplication::SendRA (Ptr<Packet> packet, uint32_t teid, uint64_t imsi)
{
  NS_LOG_FUNCTION (this << packet << teid << imsi);

  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi);
  Ipv6Address enbAddr = ueit->second->GetEnbAddr ();
  if (teid == 0)
    {
      NS_LOG_WARN ("no matching bearer for this packet");
    }
  else
    {
      SendToS1uSocket (packet, enbAddr, teid);
    }
}

void
Epc6SgwApplication::SetUePrefixCallback (Callback<void, uint64_t, Ipv6Address> &setUePrefixCallback)
{
  NS_LOG_FUNCTION (this);
  setUePrefixCallback = MakeCallback (&Epc6SgwApplication::SetUePrefix, this);
}

void
Epc6SgwApplication::SetSendRACallback (Callback<void, Ptr<Packet>, uint32_t, uint64_t> &sendRACallback)
{
  NS_LOG_FUNCTION (this);
  sendRACallback = MakeCallback (&Epc6SgwApplication::SendRA, this);
}

void
Epc6SgwApplication::SetNewHostCallback (Callback<void, uint32_t, uint64_t, uint8_t> newHostCallback)
{
  NS_LOG_FUNCTION (this);
  m_newHost = newHostCallback;
}

uint32_t
Epc6SgwApplication::GetTunnelInterfaceId ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_tunDevice->GetIfIndex ();
}

void
Epc6SgwApplication::DoCreateSessionRequest (EpcS11SapSgw::CreateSessionRequestMessage req)
{
  NS_LOG_FUNCTION (this << req.imsi);

  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (req.imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << req.imsi); 
  uint16_t cellId = req.uli.gci;
  std::map<uint16_t, EnbInfo>::iterator enbit = m_enbInfoByCellId.find (cellId);
  NS_ASSERT_MSG (enbit != m_enbInfoByCellId.end (), "unknown CellId " << cellId); 
  Ipv6Address enbAddr = enbit->second.enbAddr;
  ueit->second->SetEnbAddr (enbAddr);

  EpcS11SapMme::CreateSessionResponseMessage res;
  res.teid = req.imsi; // trick to avoid the need for allocating TEIDs on the S11 interface

  for (std::list<EpcS11SapSgw::BearerContextToBeCreated>::iterator bit = req.bearerContextsToBeCreated.begin ();
       bit != req.bearerContextsToBeCreated.end ();
       ++bit)
    {
      // simple sanity check. If you ever need more than 4M teids
      // throughout your simulation, you'll need to implement a smarter teid
      // management algorithm. 
      NS_ABORT_IF (m_teidCount == 0xFFFFFFFF);
      uint32_t teid = ++m_teidCount;  
      ueit->second->AddBearer (bit->tft, bit->epsBearerId, teid);

      EpcS11SapMme::BearerContextCreated bearerContext;
      bearerContext.sgwFteid.teid = teid;
      bearerContext.sgwFteid.isIpv4 = false;
      bearerContext.sgwFteid.address6 = enbit->second.sgwAddr;
      bearerContext.epsBearerId =  bit->epsBearerId; 
      bearerContext.bearerLevelQos = bit->bearerLevelQos; 
      bearerContext.tft = bit->tft;
      res.bearerContextsCreated.push_back (bearerContext);
    }
  m_s11SapMme->CreateSessionResponse (res);
  if (!m_newHost.IsNull ())
    m_newHost ((res.bearerContextsCreated.begin ())->sgwFteid.teid, req.imsi, 8 /* 3GPP */);
}

void 
Epc6SgwApplication::DoModifyBearerRequest (EpcS11SapSgw::ModifyBearerRequestMessage req)
{
  NS_LOG_FUNCTION (this << req.teid);
  uint64_t imsi = req.teid; // trick to avoid the need for allocating TEIDs on the S11 interface
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi); 
  uint16_t cellId = req.uli.gci;
  std::map<uint16_t, EnbInfo>::iterator enbit = m_enbInfoByCellId.find (cellId);
  NS_ASSERT_MSG (enbit != m_enbInfoByCellId.end (), "unknown CellId " << cellId); 
  Ipv6Address enbAddr = enbit->second.enbAddr;
  ueit->second->SetEnbAddr (enbAddr);
  // no actual bearer modification: for now we just support the minimum needed for path switch request (handover)
  EpcS11SapMme::ModifyBearerResponseMessage res;
  res.teid = imsi; // trick to avoid the need for allocating TEIDs on the S11 interface
  res.cause = EpcS11SapMme::ModifyBearerResponseMessage::REQUEST_ACCEPTED;
  m_s11SapMme->ModifyBearerResponse (res);
}
 
}; // namespace ns3
