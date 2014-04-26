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
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-header.h"

#include "identifier.h"
#include "ipv6-mobility.h"
#include "ipv6-mobility-header.h"
#include "ipv6-mobility-demux.h"
#include "ipv6-mobility-option.h"
#include "ipv6-mobility-option-header.h"
#include "ipv6-mobility-option-demux.h"
#include "ipv6-mobility-l4-protocol.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("Ipv6MobilityL4Protocol");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityL4Protocol);

const uint8_t Ipv6MobilityL4Protocol::PROT_NUMBER = 135;

const double Ipv6MobilityL4Protocol::MAX_BINDING_LIFETIME = (int)0xffff<<2;

const double Ipv6MobilityL4Protocol::INITIAL_BINDING_ACK_TIMEOUT_FIRSTREG = 1.5;

const double Ipv6MobilityL4Protocol::INITIAL_BINDING_ACK_TIMEOUT_REREG = 1.0;

const uint8_t Ipv6MobilityL4Protocol::MAX_BINDING_UPDATE_RETRY_COUNT = 3;

const uint32_t Ipv6MobilityL4Protocol::MIN_DELAY_BEFORE_BCE_DELETE = 10000;

const uint32_t Ipv6MobilityL4Protocol::MIN_DELAY_BEFORE_NEW_BCE_ASSIGN = 1500;

const uint32_t Ipv6MobilityL4Protocol::TIMESTAMP_VALIDITY_WINDOW = 300;

TypeId Ipv6MobilityL4Protocol::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityL4Protocol")
    .SetParent<IpL4Protocol> ()
    .AddConstructor<Ipv6MobilityL4Protocol> ()
    ;
  return tid;
}

Ipv6MobilityL4Protocol::Ipv6MobilityL4Protocol ()
  : m_node (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

Ipv6MobilityL4Protocol::~Ipv6MobilityL4Protocol ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void Ipv6MobilityL4Protocol::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  IpL4Protocol::DoDispose ();
}

void Ipv6MobilityL4Protocol::NotifyNewAggregate ()
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

void Ipv6MobilityL4Protocol::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node> Ipv6MobilityL4Protocol::GetNode (void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

int Ipv6MobilityL4Protocol::GetProtocolNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return PROT_NUMBER;
}

int Ipv6MobilityL4Protocol::GetVersion () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return 1;
}

void Ipv6MobilityL4Protocol::SendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl)
{
  NS_LOG_FUNCTION (this << packet << src << dst << (uint32_t) ttl);
  Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol> ();
  NS_ASSERT (ipv6 != 0);

  SocketIpTtlTag tag;
  tag.SetTtl (ttl);
  packet->AddPacketTag (tag);
  ipv6->Send (packet, src, dst, PROT_NUMBER, 0);
}

enum IpL4Protocol::RxStatus Ipv6MobilityL4Protocol::Receive (Ptr<Packet> packet, Ipv6Header const &header, Ptr<Ipv6Interface> incomingInterface)
{
  NS_LOG_FUNCTION (this << packet << header << incomingInterface);
  Ptr<Packet> p = packet->Copy ();
  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = GetObject<Ipv6MobilityDemux> ();
  Ptr<Ipv6Mobility> ipv6Mobility = 0;
  Ipv6MobilityHeader mh;

  p->PeekHeader (mh);
  ipv6Mobility = ipv6MobilityDemux -> GetMobility (mh.GetMhType());

  if(ipv6Mobility)
    {
      ipv6Mobility -> Process (p, header.GetSourceAddress (), header.GetDestinationAddress (), incomingInterface);
    }
  else
    {
      NS_LOG_FUNCTION( "Mobility Packet with Unknown MhType (" << (uint32_t)mh.GetMhType() << ")" );
    }

  return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus Ipv6MobilityL4Protocol::Receive (Ptr<Packet> packet, Ipv4Header const &header, Ptr<Ipv4Interface> incomingInterface)
{
  NS_LOG_FUNCTION(this << packet << header << incomingInterface);
  return IpL4Protocol::RX_ENDPOINT_UNREACH;
}

void Ipv6MobilityL4Protocol::SetDownTarget (IpL4Protocol::DownTargetCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
}

void Ipv6MobilityL4Protocol::SetDownTarget6 (IpL4Protocol::DownTargetCallback6 callback)
{
  NS_LOG_FUNCTION (this << &callback);
}

IpL4Protocol::DownTargetCallback Ipv6MobilityL4Protocol::GetDownTarget (void) const
{
  NS_LOG_FUNCTION (this);
  return (IpL4Protocol::DownTargetCallback)NULL;
}

IpL4Protocol::DownTargetCallback6 Ipv6MobilityL4Protocol::GetDownTarget6 (void) const
{
  NS_LOG_FUNCTION (this);
  return (IpL4Protocol::DownTargetCallback6)NULL;
}

void Ipv6MobilityL4Protocol::RegisterMobility()
{
  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = CreateObject<Ipv6MobilityDemux>();
  ipv6MobilityDemux->SetNode( m_node );
  
  m_node->AggregateObject( ipv6MobilityDemux );
  
  Ptr<Ipv6MobilityBindingUpdate> bu = CreateObject<Ipv6MobilityBindingUpdate>();
  bu->SetNode(m_node);
  ipv6MobilityDemux->Insert(bu);
  
  Ptr<Ipv6MobilityBindingAck> ba = CreateObject<Ipv6MobilityBindingAck>();
  ba->SetNode(m_node);
  ipv6MobilityDemux->Insert(ba);  
}

void Ipv6MobilityL4Protocol::RegisterMobilityOptions()
{
  Ptr<Ipv6MobilityOptionDemux> ipv6MobilityOptionDemux = CreateObject<Ipv6MobilityOptionDemux>();
  ipv6MobilityOptionDemux->SetNode (m_node);
  
  m_node->AggregateObject (ipv6MobilityOptionDemux);
  
  Ptr<Ipv6MobilityOptionPad1> pad1 = CreateObject<Ipv6MobilityOptionPad1>();
  pad1->SetNode (m_node);
  ipv6MobilityOptionDemux->Insert (pad1);
  
  Ptr<Ipv6MobilityOptionPadn> padn = CreateObject<Ipv6MobilityOptionPadn> ();
  padn->SetNode (m_node);
  ipv6MobilityOptionDemux->Insert (padn);
  
  //for PMIPv6
  Ptr<Ipv6MobilityOptionMobileNodeIdentifier> nai = CreateObject<Ipv6MobilityOptionMobileNodeIdentifier>();
  nai->SetNode(m_node);
  ipv6MobilityOptionDemux->Insert(nai);
  
  Ptr<Ipv6MobilityOptionHomeNetworkPrefix> hnp = CreateObject<Ipv6MobilityOptionHomeNetworkPrefix>();
  hnp->SetNode(m_node);
  ipv6MobilityOptionDemux->Insert(hnp);
  
  Ptr<Ipv6MobilityOptionHandoffIndicator> hi = CreateObject<Ipv6MobilityOptionHandoffIndicator>();
  hi->SetNode(m_node);
  ipv6MobilityOptionDemux->Insert(hi);
  
  Ptr<Ipv6MobilityOptionAccessTechnologyType> att = CreateObject<Ipv6MobilityOptionAccessTechnologyType>();
  att->SetNode(m_node);
  ipv6MobilityOptionDemux->Insert(att);
  
  Ptr<Ipv6MobilityOptionMobileNodeLinkLayerIdentifier> llid = CreateObject<Ipv6MobilityOptionMobileNodeLinkLayerIdentifier>();
  llid->SetNode(m_node);
  ipv6MobilityOptionDemux->Insert(llid);
  
  Ptr<Ipv6MobilityOptionLinkLocalAddress> lla = CreateObject<Ipv6MobilityOptionLinkLocalAddress>();
  lla->SetNode(m_node);
  ipv6MobilityOptionDemux->Insert(lla);
  
  Ptr<Ipv6MobilityOptionTimestamp> timestamp = CreateObject<Ipv6MobilityOptionTimestamp>();
  timestamp->SetNode(m_node);
  ipv6MobilityOptionDemux->Insert(timestamp);
}

} /* namespace ns3 */

