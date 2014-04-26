/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Proxy Mobile IPv6 (PMIPv6) (RFC5213) Implementation
 *
 * Copyright (c) 2010 KUT, ETRI
 * (Korea Univerity of Technology and Education)
 * (Electronics and Telecommunications Research Institute)
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
 * Author: Hyon-Young Choi <commani@gmail.com>
 */

#include <stdio.h>
#include <sstream>

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/boolean.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-mac.h"
#include "ns3/regular-wifi-mac.h"
//#include "ns3/wimax-net-device.h"
#include "ns3/point-to-point-net-device.h"

#include "ns3/ipv4-header.h"
#include "ns3/ipv4-interface.h"

#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"

#include "identifier.h"
#include "ipv6-mobility-header.h"
#include "ipv6-mobility.h"

#include "ipv6-mobility-option-header.h"
#include "ipv6-mobility-option.h"

#include "ipv6-mobility-l4-protocol.h"

#include "pmipv6-mag-notifier.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("Pmipv6MagNotifier");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(Pmipv6MagNotifyHeader);

TypeId Pmipv6MagNotifyHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Pmipv6MagNotifyHeader")
    .SetParent<Header> ()
    .AddConstructor<Pmipv6MagNotifyHeader> ()
    ;
  return tid;
}

TypeId Pmipv6MagNotifyHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Pmipv6MagNotifyHeader::Pmipv6MagNotifyHeader()
 : m_nextHeader(59), /* no next header */
   m_length(1)
{
  memset(m_reserved, 0, sizeof(m_reserved));
}

Pmipv6MagNotifyHeader::~Pmipv6MagNotifyHeader()
{
}

Mac48Address Pmipv6MagNotifyHeader::GetMacAddress() const
{
  return m_macAddress;
}

void Pmipv6MagNotifyHeader::SetMacAddress(Mac48Address macaddr)
{
  m_macAddress = macaddr;
}

uint8_t Pmipv6MagNotifyHeader::GetAccessTechnologyType() const
{
  return m_accessTechnologyType;
}

void Pmipv6MagNotifyHeader::SetAccessTechnologyType(uint8_t att)
{
  m_accessTechnologyType =  att;
} 

void Pmipv6MagNotifyHeader::Print (std::ostream& os) const
{
  os << "( from: " << m_macAddress << ", ATT: " << (uint32_t)m_accessTechnologyType << ")";
}

uint32_t Pmipv6MagNotifyHeader::GetSerializedSize () const
{
  return 16;
}

void Pmipv6MagNotifyHeader::Serialize (Buffer::Iterator start) const
{
  uint8_t buf[6];
  Buffer::Iterator i = start;
  
  i.WriteU8(m_nextHeader);
  i.WriteU8(m_length);
  m_macAddress.CopyTo(buf);
  i.Write(buf, 6);
  i.WriteU8(m_accessTechnologyType);
  i.Write(m_reserved, sizeof(m_reserved));
}

uint32_t Pmipv6MagNotifyHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t buf[6];
  Buffer::Iterator i = start;
  
  m_nextHeader = i.ReadU8();
  m_length = i.ReadU8();
  i.Read(buf, 6);
  m_macAddress.CopyFrom(buf);
  m_accessTechnologyType = i.ReadU8();
  i.Read(m_reserved, sizeof(m_reserved));
  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Pmipv6MagNotifier);

const uint8_t Pmipv6MagNotifier::PROT_NUMBER = 250;

TypeId Pmipv6MagNotifier::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Pmipv6MagNotifier")
    .SetParent<IpL4Protocol> ()
    .AddConstructor<Pmipv6MagNotifier> ()
    ;
  return tid;
}

Pmipv6MagNotifier::Pmipv6MagNotifier ()
  : m_node (0),
    m_targetAddress ("::")
{
  NS_LOG_FUNCTION_NOARGS ();
}

Pmipv6MagNotifier::~Pmipv6MagNotifier ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void Pmipv6MagNotifier::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  IpL4Protocol::DoDispose ();
}

void Pmipv6MagNotifier::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_node == 0)
    {
      Ptr<Node> node = this->GetObject<Node> ();
      if (node != 0)
        {
          Ptr<Ipv6L3Protocol> ipv6 = this->GetObject<Ipv6L3Protocol> ();
          if (ipv6 != 0)
            {
              this->SetNode (node);
              ipv6->Insert (this);
            }
          if (!m_targetAddress.IsAny ())
            {
              // register linkup callback.
              uint32_t nDev = node->GetNDevices ();
              for (uint32_t i = 0; i < nDev; ++i) 
                {
                  Ptr<NetDevice> dev = node->GetDevice(i);
                  Ptr<WifiNetDevice> wDev = dev->GetObject<WifiNetDevice> ();
                  if (wDev)
                    {
                      Ptr<WifiMac> mac = wDev->GetMac();
                      Ptr<RegularWifiMac> rmac = mac->GetObject<RegularWifiMac> ();
                      if(!mac || !rmac)
                        {
                          continue;
                        }
                      rmac->SetNewHostCallback (MakeCallback (&Pmipv6MagNotifier::HandleNewNode, this));
                      continue;
                    }
                    
//                  Ptr<WimaxNetDevice> wDev2 = dev->GetObject<WimaxNetDevice> ();
//
//                  if (wDev2)
//                    {
//                      wDev2->SetNewHostCallback (MakeCallback (&Pmipv6MagNotifier::HandleNewNode, this));
//
//                      continue;
//                    }
                }
            }
        }
    }
  IpL4Protocol::NotifyNewAggregate ();
}

void Pmipv6MagNotifier::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node> Pmipv6MagNotifier::GetNode (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

int Pmipv6MagNotifier::GetProtocolNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return PROT_NUMBER;
}

void Pmipv6MagNotifier::SendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl)
{
  NS_LOG_FUNCTION (this << packet << src << dst << (uint32_t) ttl);
  
  Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();
  
  NS_ASSERT (ipv6 != 0 && ipv6->GetRoutingProtocol () != 0);
  
  Ipv6Header header;
  SocketIpTtlTag tag;
  Socket::SocketErrno err;
  Ptr<Ipv6Route> route;
  Ptr<NetDevice> oif (0); // specify non-zero if bound to a source address

  header.SetDestinationAddress (dst);
  route = ipv6->GetRoutingProtocol ()->RouteOutput (packet, header, oif, err);

  if (route != 0)
    {
      tag.SetTtl (ttl);
      packet->AddPacketTag (tag);
      src = route->GetSource ();
      ipv6->Send (packet, src, dst, PROT_NUMBER, route);
    }
  else
    {
      NS_LOG_LOGIC ("no route.. drop notify message");
    }  
}

enum IpL4Protocol::RxStatus Pmipv6MagNotifier::Receive (Ptr<Packet> packet, Ipv6Header const &header, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << header << interface);
  
  Ptr<Packet> p = packet->Copy ();
  Pmipv6MagNotifyHeader magNotifyHeader;
  if (!m_newNodeCallback.IsNull ())
    {
      p->RemoveHeader(magNotifyHeader);
      m_newNodeCallback (magNotifyHeader.GetMacAddress (),
                         Mac48Address::ConvertFrom (interface->GetDevice ()->GetAddress ()),
                         magNotifyHeader.GetAccessTechnologyType ());
    }
  return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus Pmipv6MagNotifier::Receive (Ptr<Packet> p, Ipv4Header const &header, Ptr<Ipv4Interface> incomingInterface)
{
  NS_LOG_FUNCTION(this << p << header << incomingInterface);
  return IpL4Protocol::RX_ENDPOINT_UNREACH;
}

void Pmipv6MagNotifier::SetDownTarget (IpL4Protocol::DownTargetCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
}

void Pmipv6MagNotifier::SetDownTarget6 (IpL4Protocol::DownTargetCallback6 callback)
{
  NS_LOG_FUNCTION (this << &callback);
}

IpL4Protocol::DownTargetCallback Pmipv6MagNotifier::GetDownTarget (void) const
{
  NS_LOG_FUNCTION (this);
  return (IpL4Protocol::DownTargetCallback)NULL;
}

IpL4Protocol::DownTargetCallback6 Pmipv6MagNotifier::GetDownTarget6 (void) const
{
  NS_LOG_FUNCTION (this);
  return (IpL4Protocol::DownTargetCallback6)NULL;
}

void Pmipv6MagNotifier::SetTargetAddress(Ipv6Address target)
{
  m_targetAddress = target;
}

Ipv6Address Pmipv6MagNotifier::GetTargetAddress()
{
  return m_targetAddress;
}

void Pmipv6MagNotifier::SetNewNodeCallback (Callback<void, Mac48Address, Mac48Address, uint8_t> cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_newNodeCallback = cb;
}

void Pmipv6MagNotifier::HandleNewNode(Mac48Address from, Mac48Address to, uint8_t att)
{
  NS_LOG_FUNCTION (this << from << to << (uint32_t) att );
  
  Ptr<Packet> p = Create<Packet>();
  Pmipv6MagNotifyHeader header;
  header.SetMacAddress(from);
  header.SetAccessTechnologyType(att);
  p->AddHeader(header);
  SendMessage (p, Ipv6Address::GetAny (), m_targetAddress, 64);
}

} /* namespace ns3 */

