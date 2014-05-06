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
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-interface.h"

#include "pmipv6-profile.h"
#include "ipv6-mobility-header.h"
#include "pmipv6-agent.h"
#include "ipv6-mobility-l4-protocol.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("Pmipv6Agent");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Pmipv6Agent);

TypeId Pmipv6Agent::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Pmipv6Agent")
    .SetParent<Object> ()
    .AddConstructor<Pmipv6Agent> ()
    ;
  return tid;
}

Pmipv6Agent::Pmipv6Agent ()
  : m_node (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

Pmipv6Agent::~Pmipv6Agent ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void Pmipv6Agent::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  m_profile = 0;
  Object::DoDispose ();
}

void Pmipv6Agent::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node> Pmipv6Agent::GetNode (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

Ptr<Pmipv6Profile> Pmipv6Agent::GetProfile() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_profile;
}

void Pmipv6Agent::SetProfile(Ptr<Pmipv6Profile> pf)
{
  NS_LOG_FUNCTION (this << pf);
  
  m_profile = pf;
}

uint8_t Pmipv6Agent::Receive (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION ( this << packet << src << dst << interface );
  
  Ptr<Packet> p = packet->Copy ();
  Ipv6MobilityHeader mh;
  p->PeekHeader (mh);
  uint8_t mhType = mh.GetMhType ();
  
  if (mhType == Ipv6MobilityHeader::IPV6_MOBILITY_BINDING_UPDATE)
    {
	  Ipv6MobilityBindingUpdateHeader mj;
	  p->PeekHeader (mj);
	  if(!(mj.GetFlagT()))
      	HandlePbu (packet, src, dst, interface);
	  else{
	  		  NS_LOG_LOGIC ("Handler for HUA called");
	  		  HandleHua(packet, src, dst, interface);
	  	  }
    }
  else if (mhType == Ipv6MobilityHeader::IPV6_MOBILITY_BINDING_ACKNOWLEDGEMENT)
    {
	  Ipv6MobilityBindingAckHeader mj;
	  p->PeekHeader (mj);
	  if(!(mj.GetFlagT()))
		  HandlePba (packet, src, dst, interface);
	  else{
		  NS_LOG_LOGIC ("Handler for HUR called");
		  HandleHur(packet, src, dst, interface);
	  }
    }
  else
    {
      NS_LOG_ERROR ("Unknown MHType (" << (uint32_t) mhType << ")");
    }
  return 0;
}

void Pmipv6Agent::SendMessage (Ptr<Packet> packet, Ipv6Address dst, uint32_t ttl)
{
  NS_LOG_FUNCTION (this << packet << dst << (uint32_t)ttl);
  
  Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();
  NS_ASSERT (ipv6 != 0 && ipv6->GetRoutingProtocol () != 0);
  NS_LOG_FUNCTION(packet->GetSerializedSize());
  Ipv6Header header;
  SocketIpTtlTag tag;
  Socket::SocketErrno err;
  Ptr<Ipv6Route> route;
  Ptr<NetDevice> oif (0); //specify non-zero if bound to a source address

  header.SetDestinationAddress (dst);
  route = ipv6->GetRoutingProtocol ()->RouteOutput (packet, header, oif, err);

  if (route != 0)
    {
      tag.SetTtl (ttl);
      packet->AddPacketTag (tag);
      Ipv6Address src = route->GetSource ();
      ipv6->Send (packet, src, dst, Ipv6MobilityL4Protocol::PROT_NUMBER, route);
    }
  else
    {
      NS_LOG_LOGIC ("no route.. drop pmipv6 message");
    }
}

uint8_t Pmipv6Agent::HandlePbu (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION ( this << src << dst );
  NS_LOG_WARN ("No handler for PBU message");
  return 0;
}

uint8_t Pmipv6Agent::HandlePba (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION ( this << src << dst );
  NS_LOG_WARN ("No handler for PBA message");
  return 0;
}
uint8_t Pmipv6Agent::HandleHur (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION ( this << src << dst );
  NS_LOG_WARN ("No handler for HUR message");
  return 0;
}
uint8_t Pmipv6Agent::HandleHua (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION ( this << src << dst );
  NS_LOG_WARN ("No handler for HUA message");
  return 0;
}
} /* namespace ns3 */

