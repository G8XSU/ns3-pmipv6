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

#ifndef IPV6_MOBILITY_OPTION_HEADER_H
#define IPV6_MOBILITY_OPTION_HEADER_H

#include <list>

#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/identifier.h"

namespace ns3
{

/**
 * \class Ipv6MobilityOptionHeader
 * \brief Ipv6 Mobility option header.
 */
class Ipv6MobilityOptionHeader : public Header
{
public:
  /**
   * \struct Alignment
   * \brief represents the alignment requirements of an option header
   *
   * Represented ad facter*n+offset (eg. 8n+2) See RFC 2460.
   * No alignment is represented as 1n+0
   */
   struct Alignment
   {
     uint8_t factor;
	 uint8_t offset;
   };
  
  /**
   * \brief Get the UID of this class.
   * \return UID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the instance type ID.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId () const;

  /**
   * \brief Constructor.
   */
  Ipv6MobilityOptionHeader ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionHeader ();

  /**
   * \brief Get the type of the option.
   * \return type
   */
  uint8_t GetType () const;

  /**
   * \brief Set the type of the option.
   * \param type the type to set
   */
  void SetType (uint8_t type);

  /**
   * \brief Get the length of the option in 8 bytes unit.
   * \return length of the option
   */
  uint8_t GetLength () const;

  /**
   * \brief Set the length of the option.
   * \param len length value to set
   */
  void SetLength (uint8_t len);

  /**
   * \brief Print informations.
   * \param os output stream
   */
  virtual void Print (std::ostream& os) const;

  /**
   * \brief Get the serialized size.
   * \return serialized size
   */
  virtual uint32_t GetSerializedSize () const;

  /**
   * \brief Serialize the packet.
   * \param start start offset
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start start offset
   * \return length of packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);
  
  /**
   * \brief Get the Alignment requirement of this option header
   * \return the required alignment
   *
   * Subclasses should only implement this method, if special alignment is
   * required. Default is no alignment (1n+0)
   */
  virtual Alignment GetAlignment () const;

private:
  /**
   * \brief The type.
   */
  uint8_t m_type;

  /**
   * \brief The length.
   */
  uint8_t m_len;
};

class Ipv6MobilityOptionPad1Header : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionPad1Header();
  
  virtual ~Ipv6MobilityOptionPad1Header();

  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  
protected:

private:

};

class Ipv6MobilityOptionPadnHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionPadnHeader( uint8_t length = 2 );
  
  virtual ~Ipv6MobilityOptionPadnHeader();

  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
 
protected:

private:

};

class Ipv6MobilityOptionMobileNodeIdentifierHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionMobileNodeIdentifierHeader();
  
  virtual ~Ipv6MobilityOptionMobileNodeIdentifierHeader();
  
  uint8_t GetSubtype() const;
  void SetSubtype(uint8_t subtype);
  
  Identifier GetNodeIdentifier() const;
  void SetNodeIdentifier(Identifier id);

  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
 
protected:

private:
  uint8_t m_subtype;
  Identifier m_nodeIdentifier;
};

class Ipv6MobilityOptionHomeNetworkPrefixHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionHomeNetworkPrefixHeader();
  Ipv6MobilityOptionHomeNetworkPrefixHeader(Ipv6Address prefix, uint8_t plen);
  
  virtual ~Ipv6MobilityOptionHomeNetworkPrefixHeader();
  
  uint8_t GetReserved() const;
  void SetReserved(uint8_t reserved);
  
  uint8_t GetPrefixLength() const;
  void SetPrefixLength(uint8_t plen);
  
  Ipv6Address GetPrefix() const;
  void SetPrefix(Ipv6Address prefix);

  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual Alignment GetAlignment () const;
  
protected:

private:
  uint8_t m_reserved;
  uint8_t m_prefixLen;
  Ipv6Address m_prefix;
};

class Ipv6MobilityOptionHandoffIndicatorHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionHandoffIndicatorHeader(uint8_t hi = 4);
  
  virtual ~Ipv6MobilityOptionHandoffIndicatorHeader();
  
  uint8_t GetReserved() const;
  void SetReserved(uint8_t reserved);
  
  uint8_t GetHandoffIndicator() const;
  void SetHandoffIndicator(uint8_t hi);

  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  
protected:

private:
  uint8_t m_reserved;
  uint8_t m_handoffIndicator;

};

class Ipv6MobilityOptionAccessTechnologyTypeHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionAccessTechnologyTypeHeader();
  Ipv6MobilityOptionAccessTechnologyTypeHeader(uint8_t att);
  
  virtual ~Ipv6MobilityOptionAccessTechnologyTypeHeader();
  
  uint8_t GetReserved() const;
  void SetReserved(uint8_t reserved);
  
  uint8_t GetAccessTechnologyType (void) const;
  void SetAccessTechnologyType (uint8_t att);

  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  
protected:

private:
  uint8_t m_reserved;
  uint8_t m_accessTechnologyType;
};

class Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader();
  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader(Identifier llid);
  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader(uint8_t *pllid, uint8_t len);
  
  virtual ~Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader();

  uint16_t GetReserved() const;
  void SetReserved(uint16_t reserved);
  
  Identifier GetLinkLayerIdentifier() const;
  void SetLinkLayerIdentifier(Identifier llid);
  void SetLinkLayerIdentifier(uint8_t *pllid, uint8_t len);
  
  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual Alignment GetAlignment () const;
  
protected:

private:
  uint16_t m_reserved;
  Identifier m_linkLayerIdentifier;
};

class Ipv6MobilityOptionLinkLocalAddressHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionLinkLocalAddressHeader();
  Ipv6MobilityOptionLinkLocalAddressHeader(Ipv6Address lla);
  
  virtual ~Ipv6MobilityOptionLinkLocalAddressHeader();
  
  Ipv6Address GetLinkLocalAddress (void) const;
  
  void SetLinkLocalAddress(Ipv6Address lla);

  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual Alignment GetAlignment () const;
  
protected:

private:
  Ipv6Address m_linkLocalAddress;
};

class Ipv6MobilityOptionTimestampHeader : public Ipv6MobilityOptionHeader
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Ipv6MobilityOptionTimestampHeader();
  Ipv6MobilityOptionTimestampHeader(Time timestamp);
  
  virtual ~Ipv6MobilityOptionTimestampHeader();

  Time GetTimestamp() const;
  void SetTimestamp(Time timestamp);
  
  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual Alignment GetAlignment () const;
 
protected:

private:
  /**
    * higher 48bits: seconds since 1970/01/01 00:00:00
	* lower  16bits: 1/65536 fractions of a second
	*/
  Time m_timestamp;
};

} /* namespace ns3 */

#endif /* IPV6_MOBILITY_OPTION_HEADER_H */
