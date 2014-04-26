/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/names.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/core-config.h"
#include "ns3/arp-l3-protocol.h"
#include "ns3/ipv6-mobility-l4-protocol.h"
#include "ns3/ipv6-mobility-header.h"
#include "ns3/ipv6-mobility.h"
#include "ns3/ipv6-mobility-option-header.h"
#include "ns3/ipv6-mobility-option.h"
#include "ns3/pmipv6-mag.h"
#include "ns3/pmipv6-lma.h"
#include "ns3/pmipv6-mag-notifier.h"
#include "ns3/pmipv6-profile.h"
#include "ns3/identifier.h"
#include "ns3/ipv6-tunnel-l4-protocol.h"
#include "ns3/pmipv6-prefix-pool.h"

#include "ns3/ipv6-list-routing.h"
#include "ns3/ipv6-static-source-routing.h"

#include "pmipv6-helper.h"
#include <limits>
#include <map>

NS_LOG_COMPONENT_DEFINE ("Pmipv6Helper");

namespace ns3 {

Pmipv6LmaHelper::Pmipv6LmaHelper()
 : m_profile(0),
   m_prefixBegin("3ffe:1:4::"),
   m_prefixBeginLen(48)
{
}

Pmipv6LmaHelper::~Pmipv6LmaHelper()
{
  m_profile = 0;
}

void
Pmipv6LmaHelper::Install (Ptr<Node> node) const
{
  Ptr<Ipv6MobilityL4Protocol> mipv6 = node->GetObject<Ipv6MobilityL4Protocol>();
  if (mipv6 == 0)
    {
      mipv6 = CreateObject<Ipv6MobilityL4Protocol>();
      node->AggregateObject(mipv6);
      mipv6 = node->GetObject<Ipv6MobilityL4Protocol>();
      mipv6->RegisterMobility();
      mipv6->RegisterMobilityOptions();
    }
	
  Ptr<Ipv6TunnelL4Protocol> ip6tunnel = node->GetObject<Ipv6TunnelL4Protocol>();
  if (ip6tunnel == 0)
    {
      ip6tunnel = CreateObject<Ipv6TunnelL4Protocol>();
      node->AggregateObject (ip6tunnel);
    }
  
  Ptr<Pmipv6Lma> lma = CreateObject<Pmipv6Lma>();
  if (m_profile != 0)
    {
      lma->SetProfile (m_profile->GetProfile());
    }
  else
    {
      lma->SetProfile (CreateObject<Pmipv6Profile> ());
    }
  lma->SetPrefixPool (Create<Pmipv6PrefixPool> (m_prefixBegin, m_prefixBeginLen));
  node->AggregateObject(lma);
}

void Pmipv6LmaHelper::SetProfileHelper(Ptr<Pmipv6ProfileHelper> pf)
{
  m_profile = pf;
}
  
void Pmipv6LmaHelper::SetPrefixPoolBase(Ipv6Address prefixBegin, uint8_t prefixLen)
{
  m_prefixBegin = prefixBegin;
  m_prefixBeginLen = prefixLen;
}

Pmipv6MagHelper::Pmipv6MagHelper()
: m_profile(0)
{
}

Pmipv6MagHelper::~Pmipv6MagHelper()
{
  m_profile = 0;
}

void
Pmipv6MagHelper::Install (Ptr<Node> node, bool isLteMag) const
{
  Ptr<Ipv6MobilityL4Protocol> mipv6 = node->GetObject<Ipv6MobilityL4Protocol>();
  if(mipv6 == 0)
    {
      mipv6 = CreateObject<Ipv6MobilityL4Protocol>();
      node->AggregateObject(mipv6);
      mipv6 = node->GetObject<Ipv6MobilityL4Protocol>();
      mipv6->RegisterMobility();
      mipv6->RegisterMobilityOptions();
    }

  Ptr<Ipv6TunnelL4Protocol> ip6tunnel = node->GetObject<Ipv6TunnelL4Protocol>();
  if (ip6tunnel == 0)
    {
      ip6tunnel = CreateObject<Ipv6TunnelL4Protocol>();
      node->AggregateObject (ip6tunnel);
    }

  Ptr<Pmipv6Mag> mag = CreateObject<Pmipv6Mag>();
  mag->UseRemoteAP (false);
  mag->SetLteMag (isLteMag);
  if(m_profile != 0)
    {
      mag->SetProfile (m_profile->GetProfile ());
    }
  else
    {
      mag->SetProfile (CreateObject<Pmipv6Profile> ());
    }
	
  // Attach static source routing
  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
  NS_ASSERT_MSG(ipv6, "Install Internet-stack first before installing PMIPv6-related agents");
  Ptr<Ipv6RoutingProtocol> routingProtocol = ipv6->GetRoutingProtocol();
  Ptr<Ipv6ListRouting> listRouting = DynamicCast<Ipv6ListRouting>(routingProtocol);
  NS_ASSERT_MSG( listRouting, "PMIPv6 needs Ipv6-list-routing protocol for operation");
  Ptr<Ipv6StaticSourceRouting> sourceRouting = CreateObject<Ipv6StaticSourceRouting>();
  listRouting->AddRoutingProtocol(sourceRouting, 10); // higher priority than static routing
  node->AggregateObject(mag);
}

void
Pmipv6MagHelper::Install (Ptr<Node> node, Ipv6Address target, NodeContainer aps) const
{
  Ptr<Ipv6MobilityL4Protocol> mipv6 = node->GetObject<Ipv6MobilityL4Protocol>();
  if (mipv6 == 0)
    {
      mipv6 = CreateObject<Ipv6MobilityL4Protocol>();
      node->AggregateObject(mipv6);
      mipv6 = node->GetObject<Ipv6MobilityL4Protocol>();
      mipv6->RegisterMobility();
      mipv6->RegisterMobilityOptions();
    }

  Ptr<Ipv6TunnelL4Protocol> ip6tunnel = node->GetObject<Ipv6TunnelL4Protocol>();
  if (ip6tunnel == 0)
    {
      ip6tunnel = CreateObject<Ipv6TunnelL4Protocol>();
      node->AggregateObject (ip6tunnel);
    }
	
  // setup notifier receiver
  Ptr<Pmipv6MagNotifier> noti = CreateObject<Pmipv6MagNotifier>();
  node->AggregateObject(noti);
  
  // setup notifier sender
  for (NodeContainer::Iterator i = aps.Begin (); i != aps.End (); ++i)
    {
      noti = CreateObject<Pmipv6MagNotifier> ();
      noti->SetTargetAddress (target);
      (*i)->AggregateObject (noti);
    }

  Ptr<Pmipv6Mag> mag = CreateObject<Pmipv6Mag> ();
  mag->UseRemoteAP (true);
  if (m_profile != 0)
    {
      mag->SetProfile (m_profile->GetProfile ());
    }
  else
    {
      mag->SetProfile (CreateObject<Pmipv6Profile> ());
    }

  // Attach static source routing
  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
  NS_ASSERT_MSG (ipv6, "Install Internet-stack first before installing PMIPv6-related agents");
  Ptr<Ipv6RoutingProtocol> routingProtocol = ipv6->GetRoutingProtocol ();
  Ptr<Ipv6ListRouting> listRouting = DynamicCast<Ipv6ListRouting> (routingProtocol);
  NS_ASSERT_MSG (listRouting, "PMIPv6 needs Ipv6-list-routing protocol for operation");
  Ptr<Ipv6StaticSourceRouting> sourceRouting = CreateObject<Ipv6StaticSourceRouting> ();
  listRouting->AddRoutingProtocol (sourceRouting, 10); //higher priority than static routing
  node->AggregateObject(mag);
}

void 
Pmipv6MagHelper::SetProfileHelper(Ptr<Pmipv6ProfileHelper> pf)
{
  m_profile = pf;
}

Pmipv6ProfileHelper::Pmipv6ProfileHelper()
{
  m_profile = CreateObject<Pmipv6Profile>();
}

Pmipv6ProfileHelper::~Pmipv6ProfileHelper()
{
}

Ptr<Pmipv6Profile> Pmipv6ProfileHelper::GetProfile()
{
  return m_profile;
}

void Pmipv6ProfileHelper::AddProfile(Identifier mnId, Identifier mnLinkId, Ipv6Address lmaa, std::list<Ipv6Address> hnps, uint64_t imsi)
{
  Pmipv6Profile::Entry *entry;
  
  entry = m_profile->AddMnId (mnId);
  
  entry->SetMnIdentifier(mnId);
  entry->SetMnLinkIdentifier(mnLinkId);
  entry->SetLmaAddress(lmaa);
  entry->SetHomeNetworkPrefixes(hnps);
  entry->SetImsi (imsi);
  
  if (!mnLinkId.IsEmpty ())
    m_profile->AddMnLinkId (mnLinkId, entry);
  if (imsi != 0)
    m_profile->AddImsi (imsi, entry);
}

} // namespace ns3
