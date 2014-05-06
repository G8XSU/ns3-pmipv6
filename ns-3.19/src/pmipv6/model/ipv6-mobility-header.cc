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

#include "identifier.h"
#include "ipv6-mobility-header.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityHeader);

NS_LOG_COMPONENT_DEFINE ("Ipv6MobilityHeader");

TypeId Ipv6MobilityHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityHeader")
    .SetParent<Header> ()
    .AddConstructor<Ipv6MobilityHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityHeader::Ipv6MobilityHeader ()
  : m_payload_proto(59),
  m_header_len(0),
  m_mh_type(0),
  m_reserved(0),
  m_checksum(0)
{
}

Ipv6MobilityHeader::~Ipv6MobilityHeader ()
{
}

uint8_t Ipv6MobilityHeader::GetPayloadProto () const
{
  return m_payload_proto;
}

void Ipv6MobilityHeader::SetPayloadProto (uint8_t payload_proto)
{
  m_payload_proto = payload_proto;
}

uint8_t Ipv6MobilityHeader::GetHeaderLen () const
{
  return m_header_len;
}

void Ipv6MobilityHeader::SetHeaderLen (uint8_t header_len)
{
  m_header_len = header_len;
}

uint8_t Ipv6MobilityHeader::GetMhType () const
{
  return m_mh_type;
}

void Ipv6MobilityHeader::SetMhType (uint8_t mh_type)
{
  m_mh_type = mh_type;
}

uint8_t Ipv6MobilityHeader::GetReserved () const
{
  return m_reserved;
}

void Ipv6MobilityHeader::SetReserved (uint8_t reserved)
{
  m_reserved = reserved;
}

uint16_t Ipv6MobilityHeader::GetChecksum () const
{
  return m_checksum;
}

void Ipv6MobilityHeader::SetChecksum (uint16_t checksum)
{
  m_checksum = checksum;
}

void Ipv6MobilityHeader::Print (std::ostream& os) const
{
  os << "( payload_proto = " << (uint32_t)GetPayloadProto() << " header_len = " << (uint32_t)GetHeaderLen() << " mh_type = " << (uint32_t)GetMhType() << " checksum = " << (uint32_t)GetChecksum() <<")";
}

uint32_t Ipv6MobilityHeader::GetSerializedSize () const
{
  return 6;
}

uint32_t Ipv6MobilityHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_payload_proto = i.ReadU8();
  m_header_len = i.ReadU8();
  m_mh_type = i.ReadU8();
  m_reserved = i.ReadU8();
  m_checksum = i.ReadNtohU16();

  return GetSerializedSize ();
}

void Ipv6MobilityHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_payload_proto);
  i.WriteU8 (m_header_len);
  i.WriteU8 (m_mh_type);
  i.WriteU8 (m_reserved);
  i.WriteU16 (0);
}

MobilityOptionField::MobilityOptionField (uint32_t optionsOffset)
  : m_optionData (0),
  m_optionsOffset (optionsOffset)
{
}

MobilityOptionField::~MobilityOptionField ()
{
}

uint32_t MobilityOptionField::GetSerializedSize () const
{
  return m_optionData.GetSize () + CalculatePad ((Ipv6MobilityOptionHeader::Alignment) {8,0});
}

void MobilityOptionField::Serialize (Buffer::Iterator start) const
{
  start.Write (m_optionData.Begin (), m_optionData.End ());
  uint32_t fill = CalculatePad ((Ipv6MobilityOptionHeader::Alignment) {8,0});
  
  NS_LOG_LOGIC ("fill with " << fill << " bytes padding");
  switch (fill)
    {
    case 0: return;
    case 1: Ipv6MobilityOptionPad1Header ().Serialize (start);
            return;
    default: Ipv6MobilityOptionPadnHeader (fill).Serialize (start);
             return;
    }
}

uint32_t MobilityOptionField::Deserialize (Buffer::Iterator start, uint32_t length)
{
  uint8_t buf[length];
  start.Read (buf, length);
  m_optionData = Buffer ();
  m_optionData.AddAtEnd (length);
  m_optionData.Begin ().Write (buf, length);
  return length;
}

void MobilityOptionField::AddOption (Ipv6MobilityOptionHeader const& option)
{
  NS_LOG_FUNCTION_NOARGS ();

  uint32_t pad = CalculatePad (option.GetAlignment ());
  
  NS_LOG_LOGIC ("need " << pad << " bytes padding");
  switch (pad)
    {
    case 0: break; //no padding needed
    case 1: AddOption (Ipv6MobilityOptionPad1Header ());
            break;
    default: AddOption (Ipv6MobilityOptionPadnHeader (pad));
             break;
    }

  m_optionData.AddAtEnd (option.GetSerializedSize ());
  Buffer::Iterator it = m_optionData.End ();
  it.Prev (option.GetSerializedSize ());
  option.Serialize (it);
}

uint32_t MobilityOptionField::CalculatePad (Ipv6MobilityOptionHeader::Alignment alignment) const
{
  return (alignment.offset - (m_optionData.GetSize () + m_optionsOffset)) % alignment.factor;
}

uint32_t MobilityOptionField::GetOptionsOffset ()
{
  return m_optionsOffset;
}

Buffer MobilityOptionField::GetOptionBuffer ()
{
  return m_optionData;
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityBindingUpdateHeader);

TypeId Ipv6MobilityBindingUpdateHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityBindingUpdateHeader")
    .SetParent<Ipv6MobilityHeader> ()
    .AddConstructor<Ipv6MobilityBindingUpdateHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityBindingUpdateHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityBindingUpdateHeader::Ipv6MobilityBindingUpdateHeader ()
: MobilityOptionField(12)
{
  SetHeaderLen(0);
  SetMhType(IPV6_MOBILITY_BINDING_UPDATE);
  SetReserved(0);
  SetChecksum(0);
  
  SetSequence(0);
  SetFlagA(0);
  SetFlagH(0);
  SetFlagL(0);
  SetFlagK(0);
  SetFlagM(0);
  SetFlagR(0);
  SetFlagP(0);
  SetReserved2(0);
  SetLifetime(0);
}

Ipv6MobilityBindingUpdateHeader::~Ipv6MobilityBindingUpdateHeader ()
{
}

uint16_t Ipv6MobilityBindingUpdateHeader::GetSequence () const
{
  return m_sequence;
}

void Ipv6MobilityBindingUpdateHeader::SetSequence (uint16_t sequence)
{
  m_sequence = sequence;
}

bool Ipv6MobilityBindingUpdateHeader::GetFlagA () const
{
  return m_flagA;
}

void Ipv6MobilityBindingUpdateHeader::SetFlagA (bool a)
{
  m_flagA = a;
}

bool Ipv6MobilityBindingUpdateHeader::GetFlagH () const
{
  return m_flagH;
}

void Ipv6MobilityBindingUpdateHeader::SetFlagH (bool h)
{
  m_flagH = h;
}

bool Ipv6MobilityBindingUpdateHeader::GetFlagL () const
{
  return m_flagL;
}

void Ipv6MobilityBindingUpdateHeader::SetFlagL (bool l)
{
  m_flagL = l;
}

bool Ipv6MobilityBindingUpdateHeader::GetFlagK () const
{
  return m_flagK;
}

void Ipv6MobilityBindingUpdateHeader::SetFlagK (bool k)
{
  m_flagK = k;
}

bool Ipv6MobilityBindingUpdateHeader::GetFlagM () const
{
  return m_flagM;
}

void Ipv6MobilityBindingUpdateHeader::SetFlagM (bool m)
{
  m_flagM = m;
}

bool Ipv6MobilityBindingUpdateHeader::GetFlagR () const
{
  return m_flagR;
}

void Ipv6MobilityBindingUpdateHeader::SetFlagR (bool r)
{
  m_flagR = r;
}

bool Ipv6MobilityBindingUpdateHeader::GetFlagP () const
{
  return m_flagP;
}

void Ipv6MobilityBindingUpdateHeader::SetFlagP (bool p)
{
  m_flagP = p;
}
bool Ipv6MobilityBindingUpdateHeader::GetFlagT () const
{
  return m_flagT;
}
void Ipv6MobilityBindingUpdateHeader::SetFlagT (bool t)
{
  m_flagT = t;
}
uint16_t Ipv6MobilityBindingUpdateHeader::GetReserved2 () const
{
  return m_reserved2;
}

void Ipv6MobilityBindingUpdateHeader::SetReserved2 (uint16_t reserved2)
{
  m_reserved2 = reserved2;
}

uint16_t Ipv6MobilityBindingUpdateHeader::GetLifetime () const
{
  return m_lifetime;
}

void Ipv6MobilityBindingUpdateHeader::SetLifetime (uint16_t lifetime)
{
  m_lifetime = lifetime;
}

void Ipv6MobilityBindingUpdateHeader::Print (std::ostream& os) const
{
  os << "( payload_proto = " << (uint32_t)GetPayloadProto() << " header_len = " << (uint32_t)GetHeaderLen() << " mh_type = " << (uint32_t)GetMhType() << " checksum = " << (uint32_t)GetChecksum();
  os << " sequence = " << (uint32_t)GetSequence () << ")";
}

uint32_t Ipv6MobilityBindingUpdateHeader::GetSerializedSize () const
{
  return 12 + MobilityOptionField::GetSerializedSize();
}

void Ipv6MobilityBindingUpdateHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  uint32_t reserved2 = m_reserved2;

  i.WriteU8 (GetPayloadProto());
  
  i.WriteU8 ((uint8_t) (( GetSerializedSize() >> 3) - 1));
  i.WriteU8 (GetMhType());
  i.WriteU8 (GetReserved());
  
  i.WriteU16 (0);
  
  i.WriteHtonU16 (m_sequence);
  
  if (m_flagA) {
    reserved2 |= (uint16_t)(1 << 15);
  }
  
  if (m_flagH) {
    reserved2 |= (uint16_t)(1 << 14);
  }
  
  if (m_flagL) {
    reserved2 |= (uint16_t)(1 << 13);
  }
  
  if (m_flagK) {
    reserved2 |= (uint16_t)(1 << 12);
  }
  
  if (m_flagM) {
    reserved2 |= (uint16_t)(1 << 11);
  }
  
  if (m_flagR) {
    reserved2 |= (uint16_t)(1 << 10);
  }
  
  if (m_flagP) {
    reserved2 |= (uint16_t)(1 << 9);
  }
  if (m_flagT) {
      reserved2 |= (uint16_t)(1 << 7);
    }
  i.WriteHtonU16 (reserved2);
  i.WriteHtonU16 (m_lifetime);
  
  MobilityOptionField::Serialize(i);
}

uint32_t Ipv6MobilityBindingUpdateHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  SetPayloadProto(i.ReadU8 ());
  SetHeaderLen(i.ReadU8 ());
  SetMhType(i.ReadU8 ());
  SetReserved(i.ReadU8 ());
  
  SetChecksum(i.ReadU16 ());
  
  m_sequence = i.ReadNtohU16 ();
  
  m_reserved2 = i.ReadNtohU16 ();
  
  m_flagA = false;
  m_flagH = false;
  m_flagL = false;
  m_flagK = false;
  m_flagM = false;
  m_flagR = false;
  m_flagP = false;
  m_flagT = false;
  if (m_reserved2 & (1 << 15))
    {
      m_flagA = true;
    }

  if (m_reserved2 & (1 << 14))
    {
      m_flagH = true;
    }

  if (m_reserved2 & (1 << 13))
    {
      m_flagL = true;
    }

  if (m_reserved2 & (1 << 12))
    {
      m_flagK = true;
    }

  if (m_reserved2 & (1 << 11))
    {
      m_flagM = true;
    }

  if (m_reserved2 & (1 << 10))
    {
      m_flagR = true;
    }

  if (m_reserved2 & (1 << 9))
    {
      m_flagP = true;
    }
  if (m_reserved2 & (1 << 7))
      {
        m_flagT = true;
      }
  m_lifetime = i.ReadNtohU16 ();
  
  MobilityOptionField::Deserialize(i, (( GetHeaderLen() + 1 ) << 3 ) - GetOptionsOffset() );
  
  return GetSerializedSize ();
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityBindingAckHeader);

TypeId Ipv6MobilityBindingAckHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityBindingAckHeader")
    .SetParent<Ipv6MobilityHeader> ()
    .AddConstructor<Ipv6MobilityBindingAckHeader> ()
    ;
  return tid;
}

TypeId Ipv6MobilityBindingAckHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

Ipv6MobilityBindingAckHeader::Ipv6MobilityBindingAckHeader ()
: MobilityOptionField(12)
{
  SetHeaderLen(0);
  SetMhType(IPV6_MOBILITY_BINDING_ACKNOWLEDGEMENT);
  SetReserved(0);
  SetChecksum(0);
  
  SetStatus(0);
  SetFlagK(0);
  SetFlagR(0);
  SetFlagP(0);
  SetReserved2(0);
  SetSequence(0);
  SetLifetime(0);
}

Ipv6MobilityBindingAckHeader::~Ipv6MobilityBindingAckHeader ()
{
}

uint8_t Ipv6MobilityBindingAckHeader::GetStatus () const
{
  return m_status;
}

void Ipv6MobilityBindingAckHeader::SetStatus (uint8_t status)
{
  m_status = status;
}

bool Ipv6MobilityBindingAckHeader::GetFlagK () const
{
  return m_flagK;
}

void Ipv6MobilityBindingAckHeader::SetFlagK (bool k)
{
  m_flagK = k;
}

bool Ipv6MobilityBindingAckHeader::GetFlagR () const
{
  return m_flagR;
}

void Ipv6MobilityBindingAckHeader::SetFlagR (bool r)
{
  m_flagR = r;
}

bool Ipv6MobilityBindingAckHeader::GetFlagP () const
{
  return m_flagP;
}
bool Ipv6MobilityBindingAckHeader::GetFlagT () const
{
  return m_flagT;
}
void Ipv6MobilityBindingAckHeader::SetFlagT (bool t)
{
  m_flagT = t;
}
void Ipv6MobilityBindingAckHeader::SetFlagP (bool p)
{
  m_flagP = p;
}

uint8_t Ipv6MobilityBindingAckHeader::GetReserved2 () const
{
  return m_reserved2;
}

void Ipv6MobilityBindingAckHeader::SetReserved2 (uint8_t reserved2)
{
  m_reserved2 = reserved2;
}

uint16_t Ipv6MobilityBindingAckHeader::GetSequence () const
{
  return m_sequence;
}

void Ipv6MobilityBindingAckHeader::SetSequence (uint16_t sequence)
{
  m_sequence = sequence;
}

uint16_t Ipv6MobilityBindingAckHeader::GetLifetime () const
{
  return m_lifetime;
}

void Ipv6MobilityBindingAckHeader::SetLifetime (uint16_t lifetime)
{
  m_lifetime = lifetime;
}

void Ipv6MobilityBindingAckHeader::Print (std::ostream& os) const
{
  os << "( payload_proto = " << (uint32_t)GetPayloadProto() << " header_len = " << (uint32_t)GetHeaderLen() << " mh_type = " << (uint32_t)GetMhType() << " checksum = " << (uint32_t)GetChecksum();
  os << " status = " << (uint32_t)GetStatus() << " sequence = " << (uint32_t)GetSequence () << ")";
}

uint32_t Ipv6MobilityBindingAckHeader::GetSerializedSize () const
{
  return 12 + MobilityOptionField::GetSerializedSize();
}

void Ipv6MobilityBindingAckHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  uint32_t reserved2 = m_reserved2;

  i.WriteU8 (GetPayloadProto());
  
  i.WriteU8 ( (uint8_t) (( GetSerializedSize() >> 3) - 1) );
  i.WriteU8 (GetMhType());
  i.WriteU8 (GetReserved());
  i.WriteU16 (0);
  
  i.WriteU8 (m_status);
  
  if (m_flagK) {
    reserved2 |= (uint8_t)(1 << 7);
  }
  
  if (m_flagR) {
    reserved2 |= (uint8_t)(1 << 6);
  }
  
  if (m_flagP) {
    reserved2 |= (uint8_t)(1 << 5);
  }
  if (m_flagT) {
      reserved2 |= (uint8_t)(1 << 4);
    }
  i.WriteU8 (reserved2);
  i.WriteHtonU16 (m_sequence);
  i.WriteHtonU16 (m_lifetime);
  
  MobilityOptionField::Serialize(i);
}

uint32_t Ipv6MobilityBindingAckHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  SetPayloadProto(i.ReadU8 ());
  SetHeaderLen(i.ReadU8 ());
  SetMhType(i.ReadU8 ());
  SetReserved(i.ReadU8 ());
  
  SetChecksum(i.ReadU16 ());
  
  m_status = i.ReadU8 ();
  
  m_reserved2 = i.ReadU8 ();
  
  m_flagK = false;
  m_flagR = false;
  m_flagP = false;
  m_flagT = false;
  if (m_reserved2 & (1 << 7))
    {
      m_flagK = true;
    }

  if (m_reserved2 & (1 << 6))
    {
      m_flagR = true;
    }

  if (m_reserved2 & (1 << 5))
    {
      m_flagP = true;
    }
  if (m_reserved2 & (1 << 4))
      {
        m_flagT = true;
      }
  m_sequence = i.ReadNtohU16 ();
  m_lifetime = i.ReadNtohU16 ();

  MobilityOptionField::Deserialize(i, (( GetHeaderLen() + 1 ) << 3 ) - 16 );
  
  return GetSerializedSize ();
}

} /* namespace ns3 */

