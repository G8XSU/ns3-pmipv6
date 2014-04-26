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

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/identifier.h"
#include "ipv6-mobility-option.h"
#include "ipv6-mobility-option-header.h"

NS_LOG_COMPONENT_DEFINE ("Ipv6MobilityOption");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOption);

TypeId Ipv6MobilityOption::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOption")
    .SetParent<Object>()
	.AddAttribute ("MobilityOptionNumber", "The IPv6 mobility option number.",
	               UintegerValue (0),
				   MakeUintegerAccessor (&Ipv6MobilityOption::GetMobilityOptionNumber),
				   MakeUintegerChecker<uint8_t> ())
	;
  return tid;
}

Ipv6MobilityOption::~Ipv6MobilityOption()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void Ipv6MobilityOption::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  Object::DoDispose ();
}

void Ipv6MobilityOption::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ipv6MobilityOptionBundle::Ipv6MobilityOptionBundle()
 : m_mnIdentifier(),
   m_mnLinkIdentifier(),
   m_accessTechnologyType(0),
   m_handoffIndicator(0),
   m_timestamp(Seconds(0.0))
{
  NS_LOG_FUNCTION_NOARGS();
}

Identifier Ipv6MobilityOptionBundle::GetMnIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_mnIdentifier;
}

void Ipv6MobilityOptionBundle::SetMnIdentifier(Identifier mnId)
{
  NS_LOG_FUNCTION ( this << mnId );

  m_mnIdentifier = mnId;
}

Identifier Ipv6MobilityOptionBundle::GetMnLinkIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_mnLinkIdentifier;
}

void Ipv6MobilityOptionBundle::SetMnLinkIdentifier(Identifier mnLinkId)
{
  NS_LOG_FUNCTION ( this << mnLinkId );
  
  m_mnLinkIdentifier = mnLinkId;
}

std::list<Ipv6Address> Ipv6MobilityOptionBundle::GetHomeNetworkPrefixes() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_homeNetworkPrefixes;
}

void Ipv6MobilityOptionBundle::SetHomeNetworkPrefixes(std::list<Ipv6Address> hnps)
{
  NS_LOG_FUNCTION_NOARGS();
  
  m_homeNetworkPrefixes = hnps;
}

void Ipv6MobilityOptionBundle::AddHomeNetworkPrefix(Ipv6Address addr)
{
  NS_LOG_FUNCTION ( this << addr );
  
  m_homeNetworkPrefixes.push_back(addr);
}

Ipv6Address Ipv6MobilityOptionBundle::GetMagLinkAddress() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_magLinkAddress;
}

void Ipv6MobilityOptionBundle::SetMagLinkAddress(Ipv6Address lla)
{
  NS_LOG_FUNCTION ( this << lla );
  
  m_magLinkAddress = lla;
}

uint8_t Ipv6MobilityOptionBundle::GetAccessTechnologyType() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_accessTechnologyType;
}

void Ipv6MobilityOptionBundle::SetAccessTechnologyType(uint8_t att)
{
  NS_LOG_FUNCTION ( this << (uint32_t) att );
  
  m_accessTechnologyType = att;
}

uint8_t Ipv6MobilityOptionBundle::GetHandoffIndicator() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_handoffIndicator;
}

void Ipv6MobilityOptionBundle::SetHandoffIndicator(uint8_t hi)
{
  NS_LOG_FUNCTION ( this << hi );
  
  m_handoffIndicator = hi;
}

Time Ipv6MobilityOptionBundle::GetTimestamp() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_timestamp;
}

void Ipv6MobilityOptionBundle::SetTimestamp(Time tm)
{
  NS_LOG_FUNCTION ( this << tm );
  
  m_timestamp = tm;
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionPad1);

TypeId Ipv6MobilityOptionPad1::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionPad1")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionPad1::~Ipv6MobilityOptionPad1()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionPad1::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionPad1::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  Ipv6MobilityOptionPad1Header pad1;
  
  p->RemoveHeader(pad1);
  
  return pad1.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionPadn);

TypeId Ipv6MobilityOptionPadn::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionPadn")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionPadn::~Ipv6MobilityOptionPadn()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionPadn::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionPadn::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);

  Ipv6MobilityOptionPadnHeader padn;
  
  p->RemoveHeader(padn);
  
  return padn.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionMobileNodeIdentifier);

TypeId Ipv6MobilityOptionMobileNodeIdentifier::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionMobileNodeIdentifier")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionMobileNodeIdentifier::~Ipv6MobilityOptionMobileNodeIdentifier()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionMobileNodeIdentifier::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionMobileNodeIdentifier::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet << (uint32_t)offset);
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  Ipv6MobilityOptionMobileNodeIdentifierHeader nai;
  
  p->RemoveHeader(nai);
  
  if ( nai.GetSubtype() == 1 ) //Network Address Identifier
    {
	  bundle.SetMnIdentifier(nai.GetNodeIdentifier());
	}
  
  return nai.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionHomeNetworkPrefix);

TypeId Ipv6MobilityOptionHomeNetworkPrefix::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionHomeNetworkPrefix")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionHomeNetworkPrefix::~Ipv6MobilityOptionHomeNetworkPrefix()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionHomeNetworkPrefix::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionHomeNetworkPrefix::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  Ipv6MobilityOptionHomeNetworkPrefixHeader hnp;
  
  p->RemoveHeader(hnp);
  
  bundle.AddHomeNetworkPrefix(hnp.GetPrefix());
 
  return hnp.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionHandoffIndicator);

TypeId Ipv6MobilityOptionHandoffIndicator::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionHandoffIndicator")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionHandoffIndicator::~Ipv6MobilityOptionHandoffIndicator()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionHandoffIndicator::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionHandoffIndicator::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ipv6MobilityOptionHandoffIndicatorHeader hi;
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  p->RemoveHeader(hi);
  
  bundle.SetHandoffIndicator(hi.GetHandoffIndicator());
 
  return hi.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionAccessTechnologyType);

TypeId Ipv6MobilityOptionAccessTechnologyType::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionAccessTechnologyType")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionAccessTechnologyType::~Ipv6MobilityOptionAccessTechnologyType()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionAccessTechnologyType::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionAccessTechnologyType::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ipv6MobilityOptionAccessTechnologyTypeHeader att;
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  p->RemoveHeader(att);
  
  bundle.SetAccessTechnologyType(att.GetAccessTechnologyType());
 
  return att.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionMobileNodeLinkLayerIdentifier);

TypeId Ipv6MobilityOptionMobileNodeLinkLayerIdentifier::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionMobileNodeLinkLayerIdentifier")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionMobileNodeLinkLayerIdentifier::~Ipv6MobilityOptionMobileNodeLinkLayerIdentifier()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionMobileNodeLinkLayerIdentifier::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionMobileNodeLinkLayerIdentifier::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader mnllid;
  
  p->RemoveHeader(mnllid);
  
  bundle.SetMnLinkIdentifier(mnllid.GetLinkLayerIdentifier());
 
  return mnllid.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionLinkLocalAddress);

TypeId Ipv6MobilityOptionLinkLocalAddress::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionLinkLocalAddress")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionLinkLocalAddress::~Ipv6MobilityOptionLinkLocalAddress()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionLinkLocalAddress::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionLinkLocalAddress::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  Ipv6MobilityOptionLinkLocalAddressHeader lla;
  
  p->RemoveHeader(lla);
  
  bundle.SetMagLinkAddress(lla.GetLinkLocalAddress());
 
  return lla.GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionTimestamp);

TypeId Ipv6MobilityOptionTimestamp::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionTimestamp")
    .SetParent<Ipv6MobilityOption>()
	;
  return tid;
}

Ipv6MobilityOptionTimestamp::~Ipv6MobilityOptionTimestamp()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityOptionTimestamp::GetMobilityOptionNumber () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return OPT_NUMBER;
}

uint8_t Ipv6MobilityOptionTimestamp::Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle)
{
  NS_LOG_FUNCTION ( this << packet );
  
  Ptr<Packet> p = packet->Copy();
  
  p->RemoveAtStart(offset);
  
  Ipv6MobilityOptionTimestampHeader timestamp;
  
  p->RemoveHeader(timestamp);
  
  bundle.SetTimestamp(timestamp.GetTimestamp());
 
  return timestamp.GetSerializedSize();
}

} /* namespace ns3 */
