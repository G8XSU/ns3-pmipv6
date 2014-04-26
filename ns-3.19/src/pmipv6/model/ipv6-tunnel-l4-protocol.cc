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
#include "identifier.h"
#include "ns3/virtual-net-device.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv4-interface.h"

#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-static-routing.h"

#include "ipv6-tunnel-l4-protocol.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("Ipv6TunnelL4Protocol");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6TunnelL4Protocol);

const uint8_t Ipv6TunnelL4Protocol::PROT_NUMBER = 41; /* IPV6-in-IPv6 */

TypeId Ipv6TunnelL4Protocol::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6TunnelL4Protocol")
    .SetParent<IpL4Protocol> ()
    .AddConstructor<Ipv6TunnelL4Protocol> ()
    ;
  return tid;
}

Ipv6TunnelL4Protocol::Ipv6TunnelL4Protocol ()
  : m_node (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

Ipv6TunnelL4Protocol::~Ipv6TunnelL4Protocol ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void Ipv6TunnelL4Protocol::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  for ( TunnelMapI i = m_tunnelMap.begin(); i != m_tunnelMap.end(); i++ )
    {
      i->second = 0;
    }
  m_tunnelMap.clear();
  IpL4Protocol::DoDispose ();
}

void Ipv6TunnelL4Protocol::NotifyNewAggregate ()
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
        }
    }
  IpL4Protocol::NotifyNewAggregate ();
}

void Ipv6TunnelL4Protocol::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node> Ipv6TunnelL4Protocol::GetNode (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

int Ipv6TunnelL4Protocol::GetProtocolNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return PROT_NUMBER;
}

void Ipv6TunnelL4Protocol::SendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl)
{
  NS_LOG_FUNCTION (this << packet << src << dst << (uint32_t)ttl);
  Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();
  SocketIpTtlTag tag;
  NS_ASSERT (ipv6 != 0);
  if (packet->PeekPacketTag (tag))
    {
      tag.SetTtl (ttl);
      packet->ReplacePacketTag (tag);
    }
  else
    {
      tag.SetTtl (ttl);
      packet->AddPacketTag (tag);
    }
  ipv6->Send (packet, src, dst, PROT_NUMBER, 0);
}

enum IpL4Protocol::RxStatus Ipv6TunnelL4Protocol::Receive (Ptr<Packet> packet, Ipv6Header const &header, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << header << interface);
  
  Ptr<Ipv6L3Protocol> ipv6 = GetNode()->GetObject<Ipv6L3Protocol>();
  NS_ASSERT (ipv6 != 0);
  
  /**
   * Check whether the packet belongs to one of tunnels
   */
  Ptr<TunnelNetDevice> tdev = GetTunnelDevice (header.GetSourceAddress ());
  if (tdev == 0)
    {
      NS_LOG_DEBUG ("The packet does not associate any tunnel device");
      return IpL4Protocol::RX_OK;
    }
  
  Ptr<Packet> p = packet->Copy();
  
  Ipv6Header innerHeader;
  p->RemoveHeader(innerHeader);
  
  Ipv6Address source = innerHeader.GetSourceAddress();
  Ipv6Address destination = innerHeader.GetDestinationAddress();
  
  if (source.IsLinkLocal() ||
      destination.IsLinkLocal() ||
      destination.IsAllNodesMulticast() ||
      destination.IsAllRoutersMulticast() ||
      destination.IsAllHostsMulticast() ||
      destination.IsSolicitedMulticast())
    {
      return IpL4Protocol::RX_OK;
    }
  
  SocketIpTtlTag tag;
  if (packet->PeekPacketTag (tag))
    {
      tag.SetTtl (innerHeader.GetHopLimit() - 1);
      packet->ReplacePacketTag (tag);
    }
  else
    {
      tag.SetTtl (innerHeader.GetHopLimit() - 1);
      packet->AddPacketTag (tag);
    }
  
  // Prevent infinite loop
  Ptr<Ipv6Route> route;
  Socket::SocketErrno err;
  Ptr<NetDevice> oif (0); // specify non-zero if bound to a source address
  
  Ipv6StaticRoutingHelper routingHelper;
  Ptr<Ipv6StaticRouting> routing = routingHelper.GetStaticRouting (ipv6);
  NS_ASSERT (routing);
  route = routing->RouteOutput (p, innerHeader, oif, err);
  
  ipv6->Send (p, source, destination, innerHeader.GetNextHeader(), route);
  return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus Ipv6TunnelL4Protocol::Receive (Ptr<Packet> p, Ipv4Header const &header, Ptr<Ipv4Interface> incomingInterface)
{
  NS_LOG_FUNCTION(this << p << header << incomingInterface);
  return IpL4Protocol::RX_ENDPOINT_UNREACH;
}

void Ipv6TunnelL4Protocol::SetDownTarget (IpL4Protocol::DownTargetCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
}

void Ipv6TunnelL4Protocol::SetDownTarget6 (IpL4Protocol::DownTargetCallback6 callback)
{
  NS_LOG_FUNCTION (this << &callback);
}

IpL4Protocol::DownTargetCallback Ipv6TunnelL4Protocol::GetDownTarget (void) const
{
  NS_LOG_FUNCTION (this);
  return (IpL4Protocol::DownTargetCallback)NULL;
}

IpL4Protocol::DownTargetCallback6 Ipv6TunnelL4Protocol::GetDownTarget6 (void) const
{
  NS_LOG_FUNCTION (this);
  return (IpL4Protocol::DownTargetCallback6)NULL;
}

uint16_t Ipv6TunnelL4Protocol::AddTunnel(Ipv6Address remote, Ipv6Address local)
{
  NS_LOG_FUNCTION (this << remote << local);
  
  // Search existing tunnel device
  TunnelMapI it = m_tunnelMap.find (remote);
  Ptr<TunnelNetDevice> dev = 0;
  
  if (it == m_tunnelMap.end ())
    {
      dev = CreateObject<TunnelNetDevice> ();
      dev->SetAddress (Mac48Address::Allocate ());
      m_node->AddDevice (dev);
      m_tunnelMap.insert (std::pair<Ipv6Address, Ptr<TunnelNetDevice> > (remote, dev));
      dev->SetRemoteAddress(remote);
      dev->SetLocalAddress(local);
    }
  else
    {
      dev = it->second;
    }
    
  dev->IncreaseRefCount ();

  Ptr<Ipv6> ipv6 = m_node->GetObject<Ipv6> ();
  int32_t ifIndex = -1;
  
  ifIndex = ipv6->GetInterfaceForDevice (dev);
  if (ifIndex == -1)
    {
      ifIndex = ipv6->AddInterface (dev);
      NS_ASSERT_MSG (ifIndex >= 0, "Cannot add an IPv6 interface");
      ipv6->SetMetric (ifIndex, 1);
      ipv6->SetUp (ifIndex);
    }
  return ifIndex;
}

void Ipv6TunnelL4Protocol::RemoveTunnel(Ipv6Address remote)
{
  NS_LOG_FUNCTION (this << remote);
  Ptr<TunnelNetDevice> dev = GetTunnelDevice (remote);
  if (dev)
    {
      dev->DecreaseRefCount ();
      if (dev->GetRefCount () == 0)
        {
          TunnelMapI it = m_tunnelMap.find (remote);
          it->second = 0;
          m_tunnelMap.erase (it);
        }
    }
}

uint16_t  Ipv6TunnelL4Protocol::ModifyTunnel(Ipv6Address remote, Ipv6Address newRemote, Ipv6Address local)
{
  NS_LOG_FUNCTION (this << remote << newRemote << local);
  
  Ptr<TunnelNetDevice> dev = GetTunnelDevice (remote);
  NS_ASSERT (dev != 0);
  NS_ASSERT (dev->GetRefCount() > 0);
	  
  RemoveTunnel (remote);
  return AddTunnel (newRemote, local);
}

Ptr<TunnelNetDevice> Ipv6TunnelL4Protocol::GetTunnelDevice(Ipv6Address remote)
{
  NS_LOG_FUNCTION ( this << remote );
  
  TunnelMapI it = m_tunnelMap.find (remote);
  if (it != m_tunnelMap.end ())
    {
      return it->second;
    }
  return 0;
}
  
} /* namespace ns3 */

