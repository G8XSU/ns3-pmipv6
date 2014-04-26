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
#include "ns3/identifier.h"
#include "ns3/simulator.h"

#include "ipv6-mobility-header.h"
#include "ipv6-mobility-option-header.h"

NS_LOG_COMPONENT_DEFINE ("Ipv6MobilityOptionHeader");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionHeader);

TypeId Ipv6MobilityOptionHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionHeader")
    .SetParent<Header> ()
    .AddConstructor<Ipv6MobilityOptionHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}


Ipv6MobilityOptionHeader::Ipv6MobilityOptionHeader ()
{
  m_type = 0;
  m_len = 0;
}

Ipv6MobilityOptionHeader::~Ipv6MobilityOptionHeader ()
{
}

uint8_t Ipv6MobilityOptionHeader::GetType () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_type;
}

void Ipv6MobilityOptionHeader::SetType (uint8_t type)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_type = type;
}

uint8_t Ipv6MobilityOptionHeader::GetLength () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_len;
}

void Ipv6MobilityOptionHeader::SetLength (uint8_t len)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_len = len;
}

void Ipv6MobilityOptionHeader::Print (std::ostream& os) const
{
  os << "( type = " << (uint32_t)GetType () << " length = " << (uint32_t)GetLength () << ")";
}

uint32_t Ipv6MobilityOptionHeader::GetSerializedSize () const
{
  return m_len;
}

uint32_t Ipv6MobilityOptionHeader::Deserialize (Buffer::Iterator start)
{
  //Buffer::Iterator i = start;
  return GetSerializedSize ();
}

void Ipv6MobilityOptionHeader::Serialize (Buffer::Iterator start) const
{
  //Buffer::Iterator i = start;
}

Ipv6MobilityOptionHeader::Alignment Ipv6MobilityOptionHeader::GetAlignment () const
{
  return (Alignment){1,0};
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionPad1Header);

TypeId Ipv6MobilityOptionPad1Header::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionPad1Header")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionPad1Header> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionPad1Header::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionPad1Header::Ipv6MobilityOptionPad1Header()
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_PAD1);
  SetLength(0);
}

Ipv6MobilityOptionPad1Header::~Ipv6MobilityOptionPad1Header()
{
}

void Ipv6MobilityOptionPad1Header::Print (std::ostream& os) const
{
  os << "( type=" << GetType() << ")";
}

uint32_t Ipv6MobilityOptionPad1Header::GetSerializedSize () const
{
  return 1;
}

void Ipv6MobilityOptionPad1Header::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
}

uint32_t Ipv6MobilityOptionPad1Header::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  
  SetType(i.ReadU8());
  
  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionPadnHeader);

TypeId Ipv6MobilityOptionPadnHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionPadnHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionPadnHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionPadnHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionPadnHeader::Ipv6MobilityOptionPadnHeader(uint8_t length)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_PADN);
  SetLength(length-2);
}

Ipv6MobilityOptionPadnHeader::~Ipv6MobilityOptionPadnHeader()
{
}

void Ipv6MobilityOptionPadnHeader::Print (std::ostream& os) const
{
  os << "( type=" << GetType() << ", length(excluding TL)=" << GetLength() <<")";
}

uint32_t Ipv6MobilityOptionPadnHeader::GetSerializedSize () const
{
  return GetLength() + 2;
}

void Ipv6MobilityOptionPadnHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  uint8_t len = GetLength();

  i.WriteU8(GetType());
  i.WriteU8(len);
  
  while(len-- > 0)
    {
      i.WriteU8(0);
    }
}

uint32_t Ipv6MobilityOptionPadnHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t len;
  
  SetType(i.ReadU8());
  SetLength((len=i.ReadU8()));
  
  while(len-- > 0)
    {
	  i.ReadU8();
	}
	
  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionMobileNodeIdentifierHeader);

TypeId Ipv6MobilityOptionMobileNodeIdentifierHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionMobileNodeIdentifierHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionMobileNodeIdentifierHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionMobileNodeIdentifierHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionMobileNodeIdentifierHeader::Ipv6MobilityOptionMobileNodeIdentifierHeader()
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_MOBILE_NODE_IDENTIFIER);
  SetLength(1);
  SetSubtype(1);
}

Ipv6MobilityOptionMobileNodeIdentifierHeader::~Ipv6MobilityOptionMobileNodeIdentifierHeader()
{
}

uint8_t Ipv6MobilityOptionMobileNodeIdentifierHeader::GetSubtype() const
{
  return m_subtype;
}

void Ipv6MobilityOptionMobileNodeIdentifierHeader::SetSubtype(uint8_t subtype)
{
  m_subtype = subtype;
}

Identifier Ipv6MobilityOptionMobileNodeIdentifierHeader::GetNodeIdentifier() const
{
  return m_nodeIdentifier;
}

void Ipv6MobilityOptionMobileNodeIdentifierHeader::SetNodeIdentifier(Identifier id)
{
  m_nodeIdentifier = id;
  
  SetLength(id.GetLength() + 1); //including subtype size
}

void Ipv6MobilityOptionMobileNodeIdentifierHeader::Print (std::ostream& os) const
{
  os << "( type=" << GetType() << ", length(excluding TL)=" << GetLength() << ", subtype=" << (uint8_t)GetSubtype() << ", NodeId=" << GetNodeIdentifier() << ")";
}

uint32_t Ipv6MobilityOptionMobileNodeIdentifierHeader::GetSerializedSize () const
{
  return GetLength() + 2;
}

void Ipv6MobilityOptionMobileNodeIdentifierHeader::Serialize (Buffer::Iterator start) const
{
  uint8_t buf_id[Identifier::MAX_SIZE];
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
  i.WriteU8(GetLength());
  
  i.WriteU8(GetSubtype());
  
  m_nodeIdentifier.CopyTo(buf_id, Identifier::MAX_SIZE);
  i.Write(buf_id, m_nodeIdentifier.GetLength());
}

uint32_t Ipv6MobilityOptionMobileNodeIdentifierHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t buf_id[Identifier::MAX_SIZE];
  Buffer::Iterator i = start;
  uint8_t len;
  
  SetType(i.ReadU8());
  SetLength((len=i.ReadU8()));
  
  SetSubtype(i.ReadU8());
  len --;
  
  i.Read(buf_id, len);
  
  SetNodeIdentifier(Identifier(buf_id, len));
	
  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionHomeNetworkPrefixHeader);

TypeId Ipv6MobilityOptionHomeNetworkPrefixHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionHomeNetworkPrefixHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionHomeNetworkPrefixHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionHomeNetworkPrefixHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionHomeNetworkPrefixHeader::Ipv6MobilityOptionHomeNetworkPrefixHeader()
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_HOME_NETWORK_PREFIX);
  SetLength(18);
  
  m_reserved = 0;
  m_prefix.Set("::");
  m_prefixLen = 0;
}

Ipv6MobilityOptionHomeNetworkPrefixHeader::Ipv6MobilityOptionHomeNetworkPrefixHeader(Ipv6Address hnp, uint8_t plen)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_HOME_NETWORK_PREFIX);
  SetLength(18);
  
  m_reserved = 0;
  m_prefix = hnp;
  m_prefixLen = plen;
}

Ipv6MobilityOptionHomeNetworkPrefixHeader::~Ipv6MobilityOptionHomeNetworkPrefixHeader()
{
}

uint8_t Ipv6MobilityOptionHomeNetworkPrefixHeader::GetReserved() const
{
  return m_reserved;
}

void Ipv6MobilityOptionHomeNetworkPrefixHeader::SetReserved(uint8_t reserved)
{
  m_reserved = reserved;
}
  
uint8_t Ipv6MobilityOptionHomeNetworkPrefixHeader::GetPrefixLength() const
{
  return m_prefixLen;
}

void Ipv6MobilityOptionHomeNetworkPrefixHeader::SetPrefixLength(uint8_t plen)
{
  m_prefixLen = plen;
}
  
Ipv6Address Ipv6MobilityOptionHomeNetworkPrefixHeader::GetPrefix() const
{
  return m_prefix;
}

void Ipv6MobilityOptionHomeNetworkPrefixHeader::SetPrefix(Ipv6Address prefix)
{
  m_prefix = prefix;
}

void Ipv6MobilityOptionHomeNetworkPrefixHeader::Print (std::ostream& os) const
{
  os << "( type=" << GetType() << ", length(excluding TL)=" << GetLength() << ", prefix_len=" << GetPrefixLength() << ", prefix=" << GetPrefix() <<")";
}

uint32_t Ipv6MobilityOptionHomeNetworkPrefixHeader::GetSerializedSize () const
{
  return GetLength()+2;
}

void Ipv6MobilityOptionHomeNetworkPrefixHeader::Serialize (Buffer::Iterator start) const
{
  uint8_t buff_prefix[16];
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
  i.WriteU8(GetLength());
  
  i.WriteU8(m_reserved);
  i.WriteU8(m_prefixLen);
  
  m_prefix.Serialize(buff_prefix);
  i.Write(buff_prefix, 16);
}

uint32_t Ipv6MobilityOptionHomeNetworkPrefixHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t buff[16];
  Buffer::Iterator i = start;
  
  SetType(i.ReadU8());
  SetLength(i.ReadU8());
  
  m_reserved = i.ReadU8();
  m_prefixLen = i.ReadU8();
  
  i.Read(buff, 16);
  m_prefix.Set(buff);
  
  return GetSerializedSize();
}

Ipv6MobilityOptionHeader::Alignment Ipv6MobilityOptionHomeNetworkPrefixHeader::GetAlignment () const
{
  return (Alignment){8,4}; //8n+4
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionHandoffIndicatorHeader);

TypeId Ipv6MobilityOptionHandoffIndicatorHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionHandoffIndicatorHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionHandoffIndicatorHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionHandoffIndicatorHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionHandoffIndicatorHeader::Ipv6MobilityOptionHandoffIndicatorHeader(uint8_t hi)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_HANDOFF_INDICATOR);
  SetLength(2);
  
  m_reserved = 0;
  m_handoffIndicator = hi;
}

Ipv6MobilityOptionHandoffIndicatorHeader::~Ipv6MobilityOptionHandoffIndicatorHeader()
{
}

uint8_t Ipv6MobilityOptionHandoffIndicatorHeader::GetReserved() const
{
  return m_reserved;
}

void Ipv6MobilityOptionHandoffIndicatorHeader::SetReserved(uint8_t reserved)
{
  m_reserved = reserved;
}

uint8_t Ipv6MobilityOptionHandoffIndicatorHeader::GetHandoffIndicator() const
{
  return m_handoffIndicator;
}

void Ipv6MobilityOptionHandoffIndicatorHeader::SetHandoffIndicator(uint8_t hi)
{
  m_handoffIndicator = hi;
}

void Ipv6MobilityOptionHandoffIndicatorHeader::Print (std::ostream& os) const
{
  os << "( type=" << (uint32_t)GetType() << ", length(excluding TL)=" << (uint32_t)GetLength() << ", handoff_indicator=" << (uint32_t)GetHandoffIndicator() << ")";
}

uint32_t Ipv6MobilityOptionHandoffIndicatorHeader::GetSerializedSize () const
{
  return GetLength() + 2;
}

void Ipv6MobilityOptionHandoffIndicatorHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
  i.WriteU8(GetLength());
  
  i.WriteU8(m_reserved);
  i.WriteU8(m_handoffIndicator);
}

uint32_t Ipv6MobilityOptionHandoffIndicatorHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  
  SetType(i.ReadU8());
  SetLength(i.ReadU8());
  
  m_reserved = i.ReadU8();
  m_handoffIndicator = i.ReadU8();
  
  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionAccessTechnologyTypeHeader);

TypeId Ipv6MobilityOptionAccessTechnologyTypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionAccessTechnologyTypeHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionAccessTechnologyTypeHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionAccessTechnologyTypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionAccessTechnologyTypeHeader::Ipv6MobilityOptionAccessTechnologyTypeHeader()
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_ACCESS_TECHNOLOGY_TYPE);
  SetLength(2);
  
  m_reserved = 0;
  m_accessTechnologyType = Ipv6MobilityHeader::OPT_ATT_RESERVED;
}

Ipv6MobilityOptionAccessTechnologyTypeHeader::Ipv6MobilityOptionAccessTechnologyTypeHeader(uint8_t att)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_ACCESS_TECHNOLOGY_TYPE);
  SetLength(2);
  
  m_reserved = 0;
  m_accessTechnologyType = att;
}

Ipv6MobilityOptionAccessTechnologyTypeHeader::~Ipv6MobilityOptionAccessTechnologyTypeHeader()
{
}

uint8_t Ipv6MobilityOptionAccessTechnologyTypeHeader::GetReserved() const
{
  return m_reserved;
}

void Ipv6MobilityOptionAccessTechnologyTypeHeader::SetReserved(uint8_t reserved)
{
  m_reserved = reserved;
}

uint8_t Ipv6MobilityOptionAccessTechnologyTypeHeader::GetAccessTechnologyType() const
{
  return m_accessTechnologyType;
}

void Ipv6MobilityOptionAccessTechnologyTypeHeader::SetAccessTechnologyType(uint8_t att)
{
  m_accessTechnologyType = att;
}

void Ipv6MobilityOptionAccessTechnologyTypeHeader::Print (std::ostream& os) const
{
  os << "( type=" << (uint32_t)GetType() << ", length(excluding TL)=" << (uint32_t)GetLength() << ", access_technology_type=" << (uint32_t)GetAccessTechnologyType() << ")";
}

uint32_t Ipv6MobilityOptionAccessTechnologyTypeHeader::GetSerializedSize () const
{
  return GetLength()+2;
}

void Ipv6MobilityOptionAccessTechnologyTypeHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
  i.WriteU8(GetLength());
  
  i.WriteU8(m_reserved);
  i.WriteU8(m_accessTechnologyType);
}

uint32_t Ipv6MobilityOptionAccessTechnologyTypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  
  SetType(i.ReadU8());
  SetLength(i.ReadU8());
  
  m_reserved = i.ReadU8();
  m_accessTechnologyType = i.ReadU8();
  
  return GetSerializedSize();
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader);

TypeId Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader()
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_MOBILE_NODE_LINK_LAYER_IDENTIFIER);
  SetLength(2);
  
  m_reserved = 0;
}

Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader(Identifier llid)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_MOBILE_NODE_LINK_LAYER_IDENTIFIER);
  SetLength(2+llid.GetLength());
  
  m_reserved = 0;
  m_linkLayerIdentifier = llid;
}

Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader(uint8_t *pllid, uint8_t len)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_MOBILE_NODE_LINK_LAYER_IDENTIFIER);
  SetLength(2+len);
  
  m_reserved = 0;
  m_linkLayerIdentifier.CopyFrom(pllid, len);
}

Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::~Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader()
{
}

uint16_t Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::GetReserved() const
{
  return m_reserved;
}

void Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::SetReserved(uint16_t reserved)
{
  m_reserved = reserved;
}

Identifier Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::GetLinkLayerIdentifier() const
{
  return m_linkLayerIdentifier;
}

void Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::SetLinkLayerIdentifier(Identifier llid)
{
  SetLength(2 + llid.GetLength());
  m_linkLayerIdentifier = llid;
}

void Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::SetLinkLayerIdentifier(uint8_t *pllid, uint8_t len)
{
  SetLength(2 + len);
  m_linkLayerIdentifier.CopyFrom(pllid, len);
}

void Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::Print (std::ostream& os) const
{
  os << "( type=" << (uint32_t)GetType() << ", length(excluding TL)=" << (uint32_t)GetLength() << ", linkLayerIdentifier=" << GetLinkLayerIdentifier() << ")";
}

uint32_t Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::GetSerializedSize () const
{
  return GetLength()+2;
}

void Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::Serialize (Buffer::Iterator start) const
{
  uint8_t buf_llid[Identifier::MAX_SIZE];
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
  i.WriteU8(GetLength());
  
  i.WriteHtonU16(m_reserved);
  
  m_linkLayerIdentifier.CopyTo(buf_llid, Identifier::MAX_SIZE);
  i.Write(buf_llid, m_linkLayerIdentifier.GetLength());
}

uint32_t Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t buf_llid[Identifier::MAX_SIZE];
  Buffer::Iterator i = start;
  uint8_t len; 
  
  SetType(i.ReadU8());
  SetLength((len=i.ReadU8()));
  
  m_reserved = i.ReadNtohU16();
  len -= 2; // decrease reserved size
  i.Read(buf_llid, len);
  m_linkLayerIdentifier.CopyFrom(buf_llid, len);
  
  return GetSerializedSize();
}

Ipv6MobilityOptionHeader::Alignment Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader::GetAlignment () const
{
  return (Alignment){2,0}; //2n
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionLinkLocalAddressHeader);

TypeId Ipv6MobilityOptionLinkLocalAddressHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionLinkLocalAddressHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionLinkLocalAddressHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionLinkLocalAddressHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionLinkLocalAddressHeader::Ipv6MobilityOptionLinkLocalAddressHeader()
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_LINK_LOCAL_ADDRESS);
  SetLength(16);
  
  m_linkLocalAddress.Set("::");
}

Ipv6MobilityOptionLinkLocalAddressHeader::Ipv6MobilityOptionLinkLocalAddressHeader(Ipv6Address lla)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_LINK_LOCAL_ADDRESS);
  SetLength(16);
  
  m_linkLocalAddress = lla;
}

Ipv6MobilityOptionLinkLocalAddressHeader::~Ipv6MobilityOptionLinkLocalAddressHeader()
{
}

Ipv6Address Ipv6MobilityOptionLinkLocalAddressHeader::GetLinkLocalAddress() const
{
  return m_linkLocalAddress;
}

void Ipv6MobilityOptionLinkLocalAddressHeader::SetLinkLocalAddress(Ipv6Address lla)
{
  m_linkLocalAddress = lla;
}

void Ipv6MobilityOptionLinkLocalAddressHeader::Print (std::ostream& os) const
{
  os << "( type=" << (uint32_t)GetType() << ", length(excluding TL)=" << (uint32_t)GetLength() << ", link_local_address=" << GetLinkLocalAddress() << ")";
}

uint32_t Ipv6MobilityOptionLinkLocalAddressHeader::GetSerializedSize () const
{
  return GetLength()+2;
}

void Ipv6MobilityOptionLinkLocalAddressHeader::Serialize (Buffer::Iterator start) const
{
  uint8_t buf_lla[16];
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
  i.WriteU8(GetLength());

  m_linkLocalAddress.Serialize(buf_lla);
  i.Write(buf_lla, 16);  
}

uint32_t Ipv6MobilityOptionLinkLocalAddressHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t buf_lla[16];
  Buffer::Iterator i = start;
  
  SetType(i.ReadU8());
  SetLength(i.ReadU8());
  
  i.Read(buf_lla, 16);
  m_linkLocalAddress.Set(buf_lla);
  
  return GetSerializedSize();
}

Ipv6MobilityOptionHeader::Alignment Ipv6MobilityOptionLinkLocalAddressHeader::GetAlignment () const
{
  return (Alignment){8,6}; //8n+6
}

NS_OBJECT_ENSURE_REGISTERED(Ipv6MobilityOptionTimestampHeader);

TypeId Ipv6MobilityOptionTimestampHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionTimestampHeader")
    .SetParent<Ipv6MobilityOptionHeader> ()
    .AddConstructor<Ipv6MobilityOptionTimestampHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityOptionTimestampHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityOptionTimestampHeader::Ipv6MobilityOptionTimestampHeader()
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_TIMESTAMP);
  SetLength(8);
  
  SetTimestamp(Simulator::Now());
}

Ipv6MobilityOptionTimestampHeader::Ipv6MobilityOptionTimestampHeader(Time timestamp)
{
  SetType(Ipv6MobilityHeader::IPV6_MOBILITY_OPT_TIMESTAMP);
  SetLength(8);
  
  SetTimestamp(timestamp);
}

Ipv6MobilityOptionTimestampHeader::~Ipv6MobilityOptionTimestampHeader()
{
}

Time Ipv6MobilityOptionTimestampHeader::GetTimestamp() const
{
  return m_timestamp;
}

void Ipv6MobilityOptionTimestampHeader::SetTimestamp(Time timestamp)
{
  m_timestamp = timestamp;
}

void Ipv6MobilityOptionTimestampHeader::Print (std::ostream& os) const
{
  os << "( type=" << (uint32_t)GetType() << ", length(excluding TL)=" << (uint32_t)GetLength() << ", timestamp=" << m_timestamp.GetSeconds() << "(s))";
}

uint32_t Ipv6MobilityOptionTimestampHeader::GetSerializedSize () const
{
  return GetLength()+2;
}

void Ipv6MobilityOptionTimestampHeader::Serialize (Buffer::Iterator start) const
{
  uint64_t timestamp;
  Buffer::Iterator i = start;

  i.WriteU8(GetType());
  i.WriteU8(GetLength());

  timestamp = m_timestamp.GetMicroSeconds();
  
  i.WriteHtonU64(timestamp);
}

uint32_t Ipv6MobilityOptionTimestampHeader::Deserialize (Buffer::Iterator start)
{
  uint64_t timestamp;
  Buffer::Iterator i = start;
  
  SetType(i.ReadU8());
  SetLength(i.ReadU8());
  
  timestamp = i.ReadNtohU64();
  m_timestamp = MicroSeconds (timestamp);
  
  return GetSerializedSize();
}

Ipv6MobilityOptionHeader::Alignment Ipv6MobilityOptionTimestampHeader::GetAlignment () const
{
  return (Alignment){8,6}; //8n+6
}

} /* namespace ns3 */
