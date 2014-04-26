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
 
#include "ns3/log.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-header.h"
#include "ns3/icmpv6-header.h"
#include "ns3/packet-socket-address.h"

#include "unicast-radvd.h"
#include "regular-unicast-radvd.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("RegularUnicastRadvd");

NS_OBJECT_ENSURE_REGISTERED (RegularUnicastRadvd);

TypeId RegularUnicastRadvd::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::RegularUnicastRadvd")
    .SetParent<UnicastRadvd> ()
    .AddConstructor<RegularUnicastRadvd> ()
    ;
  return tid;
}

RegularUnicastRadvd::RegularUnicastRadvd ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

RegularUnicastRadvd::~RegularUnicastRadvd ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void RegularUnicastRadvd::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
  UnicastRadvd::DoDispose ();
}

void RegularUnicastRadvd::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (!m_socket)
    {
      TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      NS_ASSERT (m_socket);
      m_socket->Bind ();
      m_socket->ShutdownRecv ();
    }

  for (RadvdInterfaceListCI it = m_configurations.begin () ; it != m_configurations.end () ; it++)
    {
      m_eventIds[(*it)->GetId ()] = EventId ();
      ScheduleTransmit (Seconds (0.), (*it), m_eventIds[(*it)->GetId ()], Ipv6Address::GetAllNodesMulticast (), true); 
    }
}

void RegularUnicastRadvd::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  for (EventIdMapI it = m_eventIds.begin () ; it != m_eventIds.end () ; ++it)
    {
      Simulator::Cancel ((*it).second);
    }
  m_eventIds.clear ();
}

void RegularUnicastRadvd::Send (Ptr<UnicastRadvdInterface> config, Ipv6Address dst, bool reschedule)
{
  NS_LOG_FUNCTION (this << dst);
  NS_ASSERT (m_eventIds[config->GetId ()].IsExpired ());
  
  Ipv6Header ipv6Hdr;
  
  Icmpv6RA raHdr;
  Icmpv6OptionLinkLayerAddress llaHdr;
  Icmpv6OptionMtu mtuHdr;
  Icmpv6OptionPrefixInformation prefixHdr;

  if (m_eventIds.size () == 0)
    {
      return;
    }

  std::list<Ptr<RadvdPrefix> > prefixes = config->GetPrefixes ();
  Ptr<Packet> p = Create<Packet> ();
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();

  /* set RA header information */
  raHdr.SetFlagM (config->IsManagedFlag ());
  raHdr.SetFlagO (config->IsOtherConfigFlag ());
  raHdr.SetFlagH (config->IsHomeAgentFlag ());
  raHdr.SetCurHopLimit (config->GetCurHopLimit ());
  raHdr.SetLifeTime (config->GetDefaultLifeTime ());
  raHdr.SetReachableTime (config->GetReachableTime ());
  raHdr.SetRetransmissionTime (config->GetRetransTimer ());

  if (config->IsSourceLLAddress ())
    {
      /* Get L2 address from NetDevice */
      Address addr = ipv6->GetNetDevice (config->GetInterface ())->GetAddress ();
      llaHdr = Icmpv6OptionLinkLayerAddress (true, addr);
      p->AddHeader (llaHdr);
    }

  if (config->GetLinkMtu ())
    {
      NS_ASSERT (config->GetLinkMtu () >= 1280);
      mtuHdr = Icmpv6OptionMtu (config->GetLinkMtu ());
      p->AddHeader (mtuHdr);
    }

  /* add list of prefixes */
  for (std::list<Ptr<RadvdPrefix> >::const_iterator jt = prefixes.begin () ; jt != prefixes.end () ; jt++)
    {
      uint8_t flags = 0;
      prefixHdr = Icmpv6OptionPrefixInformation ();
      prefixHdr.SetPrefix ((*jt)->GetNetwork ());
      prefixHdr.SetPrefixLength ((*jt)->GetPrefixLength ());
      prefixHdr.SetValidTime ((*jt)->GetValidLifeTime ());
      prefixHdr.SetPreferredTime ((*jt)->GetPreferredLifeTime ());

      if ((*jt)->IsOnLinkFlag ())
        {
          flags += 1 << 7;
        }

      if ((*jt)->IsAutonomousFlag ())
        {
          flags += 1 << 6;
        }

      if ((*jt)->IsRouterAddrFlag ())
        {
          flags += 1 << 5;
        }

      prefixHdr.SetFlags (flags);

      p->AddHeader (prefixHdr);
    }

  Ipv6Address src = ipv6->GetAddress (config->GetInterface (), 0).GetAddress ();

  /* as we know interface index that will be used to send RA and 
   * we always send RA with router's link-local address, we can 
   * calculate checksum here.
   */
  raHdr.CalculatePseudoHeaderChecksum (src, dst, p->GetSize () + raHdr.GetSerializedSize (), 58 /* ICMPv6 */);
  p->AddHeader (raHdr);

  ipv6Hdr.SetSourceAddress (src);
  ipv6Hdr.SetDestinationAddress (dst);
  ipv6Hdr.SetNextHeader (58 /* ICMPv6 */);
  ipv6Hdr.SetPayloadLength (p->GetSize());
  ipv6Hdr.SetHopLimit (255);
  
  p->AddHeader (ipv6Hdr);
  
  PacketSocketAddress target;

  target.SetSingleDevice(ipv6->GetNetDevice(config->GetInterface())->GetIfIndex());
  target.SetPhysicalAddress (config->GetPhysicalAddress());
  target.SetProtocol (0x86dd /* Ipv6 */);

  Ptr<NetDevice> dev = ipv6->GetNetDevice(config->GetInterface());

  NS_LOG_LOGIC ("Netdev: " << dev << ", address: " << dev->GetAddress() );

  /* send RA */
  NS_LOG_LOGIC ("Send RA");
  m_socket->SendTo (p, 0, target);

  if (reschedule)
    {
      UniformVariable rnd;
      uint64_t delay = static_cast<uint64_t> (rnd.GetValue (config->GetMinRtrAdvInterval (), config->GetMaxRtrAdvInterval ()) + 0.5);
      NS_LOG_INFO ("Reschedule in " << delay);
      Time t = MilliSeconds (delay);
      ScheduleTransmit (t, config, m_eventIds[config->GetId ()], dst, reschedule);
    }
}

bool
RegularUnicastRadvd::IsAppStarted ()
{
  if (!m_socket)
    return false;
  return true;
}
} /* namespace ns3 */

