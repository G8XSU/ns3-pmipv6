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

#ifndef IPV6_MOBILITY_HEADER_H
#define IPV6_MOBILITY_HEADER_H

#include <list>

#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/packet.h"

#include "ipv6-mobility-option-header.h"

namespace ns3
{

class Identifier;

/**
 * \class Ipv6MobilityHeader
 * \brief Ipv6Mobility header.
 */
class Ipv6MobilityHeader : public Header
{
public:
  enum MhType_e
  {
    IPV6_MOBILITY_BINDING_REFRESH_REQUEST = 0,
    IPV6_MOBILITY_HOME_TEST_INIT,
    IPV6_MOBILITY_CARE_OF_TEST_INIT,
    IPV6_MOBILITY_HOME_TEST,
    IPV6_MOBILITY_CARE_OF_TEST,
    IPV6_MOBILITY_BINDING_UPDATE,
    IPV6_MOBILITY_BINDING_ACKNOWLEDGEMENT,
    IPV6_MOBILITY_BINDING_ERROR
  };
   
  enum OptionType_e
  {
    IPV6_MOBILITY_OPT_PAD1 = 0,
    IPV6_MOBILITY_OPT_PADN,
    IPV6_MOBILITY_OPT_BINDING_REFRESH_ADVICE,
    IPV6_MOBILITY_OPT_ALTERNATE_CARE_OF_ADDRESS,
    IPV6_MOBILITY_OPT_NONCE_INDICES,
    IPV6_MOBILITY_OPT_BINDING_AUTHORIZATION_DATA,

    IPV6_MOBILITY_OPT_MOBILE_NODE_IDENTIFIER = 8,

    /* PMIPv6 options */
    IPV6_MOBILITY_OPT_HOME_NETWORK_PREFIX = 22,
    IPV6_MOBILITY_OPT_HANDOFF_INDICATOR,
    IPV6_MOBILITY_OPT_ACCESS_TECHNOLOGY_TYPE,
    IPV6_MOBILITY_OPT_MOBILE_NODE_LINK_LAYER_IDENTIFIER = 25,
    IPV6_MOBILITY_OPT_LINK_LOCAL_ADDRESS,
    IPV6_MOBILITY_OPT_TIMESTAMP
  };

  enum BAStatus_e
  {
    BA_STATUS_BINDING_UPDATE_ACCEPTED = 0,
    BA_STATUS_REASON_UNSPECIFIED = 128,
    BA_STATUS_ADMINISTRATIVELY_PROHIBITED,
    BA_STATUS_INSUFFICIENT_RESOURCES = 130,
	
    /* PMIPv6 extended BA status */
    BA_STATUS_PROXY_REG_NOT_ENABLED = 152,
    BA_STATUS_NOT_LMA_FOR_THIS_MOBILE_NODE,
    BA_STATUS_NOT_AUTHORIZED_FOR_PROXY_REG,
    BA_STATUS_MAG_NOT_AUTHORIZED_FOR_HOME_NETWORK_PREFIX = 155,
    BA_STATUS_TIMESTAMP_MISMATCH,
    BA_STATUS_TIMESTAMP_LOWER_THAN_PREV_ACCEPTED,
    BA_STATUS_MISSING_HOME_NETWORK_PREFIX_OPTION,
    BA_STATUS_BCE_PBU_PREFIX_SET_DO_NOT_MATCH,
    BA_STATUS_MISSING_MN_IDENTIFIER_OPTION = 160,
    BA_STATUS_MISSING_HANDOFF_INDICATOR_OPTION,
    BA_STATUS_MISSING_ACCESS_TECH_TYPE_OPTION
  };
  
  enum OptionHandoffIndicator_e
  {
    OPT_HI_RESERVED = 0,
    OPT_HI_ATTACH_OVER_NEW_INTERFACE,
    OPT_HI_HANDOFF_BETWEEN_DIFFERENT_INTERFACES,
    OPT_HI_HANDOFF_BETWEEN_MAGS_FOR_SAME_INTERFACE,
    OPT_HI_HANDOFF_STATE_UNKNOWN,
    OPT_HI_STATE_NOT_CHANGED,
    OPT_NEW_ATTACHMENT
  };
  
  enum OptionAccessTechType_e
  {
    OPT_ATT_RESERVED = 0,
    OPT_ATT_VIRTUAL,
    OPT_ATT_PPP,
    OPT_ATT_IEEE_802_3,
    OPT_ATT_IEEE_802_11ABG,
    OPT_ATT_IEEE_802_16E
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
  Ipv6MobilityHeader ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityHeader ();
  
  /**
   * \brief Get the payload proto field.
   * \return payload proto (MUST BE 59)
   */
  uint8_t GetPayloadProto () const;

  /**
   * \brief Set the payload_proto.
   * \param payload_proto payload protocol
   */
  void SetPayloadProto (uint8_t payload_proto);

  /**
   * \brief Get the header len field.
   * \return the unit of 8 octets
   */
  uint8_t GetHeaderLen () const;

  /**
   * \brief Set the header_len.
   * \param header_len 8octets of header length
   */
  void SetHeaderLen (uint8_t header_len);

  /**
   * \brief Get the mh type field.
   * \return the mh type
   */
  uint8_t GetMhType () const;

  /**
   * \brief Set the mh type.
   * \param mh_type the mh type
   */
  void SetMhType (uint8_t mh_type);

  /**
   * \brief Get the reserved.
   * \return reserved
   */
  uint8_t GetReserved () const;

  /**
   * \brief Set the reserved.
   * \param reserved to set
   */
  void SetReserved (uint8_t reserved);

  /**
   * \brief Get the checksum.
   * \return checksum
   */
  uint16_t GetChecksum () const;

  /**
   * \brief Set the checksum.
   * \param checksum to set
   */
  void SetChecksum (uint16_t checksum);

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

private:
  /**
   * \brief The next header.
   */
  uint8_t m_payload_proto;

  /**
   * \brief The header length.
   */
  uint8_t m_header_len;
  
   /**
   * \brief The MH type.
   */
  uint8_t m_mh_type;
  
  /**
   * \brief Reserved.
   */
  uint8_t m_reserved;

   /**
   * \brief The checksum.
   */
  uint16_t m_checksum;

};

/**
 * \class MobilityOptionField
 * \brief Option field for an IPv6MobilityHeader
 * Enables adding options to an IPv6MobilityHeader
 *
 * Implementor's note: Make sure to add the result of
 * MobilityOptionField::GetSerializedSize () to your IPv6MobilityHeader::GetSerializedSize ()
 * return value. Call MobilityOptionField::Serialize and MobilityOptionField::Deserialize at the
 * end of your corresponding IPv6MobilityHeader methods.
 */
class MobilityOptionField
{
public:
  /**
   * \brief Constructor.
   * \param optionsOffset option offset
   */
  MobilityOptionField (uint32_t optionsOffset);

  /**
   * \brief Destructor.
   */
  ~MobilityOptionField ();

  /**
   * \brief Get the serialized size of the packet.
   * \return size
   */
  uint32_t GetSerializedSize () const;

  /**
   * \brief Serialize all added options.
   * \param start Buffer iterator
   */
  void Serialize (Buffer::Iterator start) const;

  /**
   * \brief Deserialize the packet.
   * \param start Buffer iterator
   * \param length length
   * \return size of the packet
   */
  uint32_t Deserialize (Buffer::Iterator start, uint32_t length);

  /**
   * \brief Serialize the option, prepending pad1 or padn option as necessary
   * \param option the option header to serialize
   */
  void AddOption (Ipv6MobilityOptionHeader const& option);

  /**
   * \brief Get the offset where the options begin, measured from the start of
   * the extension header.
   * \return the offset from the start of the extension header
   */
  uint32_t GetOptionsOffset ();

  /**
   * \brief Get the buffer.
   * \return buffer
   */
  Buffer GetOptionBuffer ();

private:

  /**
   * \brief Calculate padding.
   * \param alignment alignment
   */
  uint32_t CalculatePad (Ipv6MobilityOptionHeader::Alignment alignment) const;

  /**
   * \brief Data payload.
   */
  Buffer m_optionData;

  /**
   * \brief Offset.
   */
  uint32_t m_optionsOffset;
};

/**
 * \class Ipv6MobilityBindingUpdateHeader
 * \brief Ipv6 Mobility Binding Update header.
 */
class Ipv6MobilityBindingUpdateHeader : public Ipv6MobilityHeader, public MobilityOptionField
{
public:
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
  Ipv6MobilityBindingUpdateHeader ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityBindingUpdateHeader ();

  /**
   * \brief Get the Sequence field.
   * \return sequence value
   */
  uint16_t GetSequence () const;

  /**
   * \brief Set the sequence field.
   * \param sequence the sequence value
   */
  void SetSequence (uint16_t sequence);

  /**
   * \brief Get the A flag.
   * \return A flag
   */
  bool GetFlagA() const;
  
  /**
   * \brief Set the A flag.
   * \param a value
   */
  void SetFlagA(bool a);

  /**
   * \brief Get the H flag.
   * \return H flag
   */
  bool GetFlagH() const;
  
  /**
   * \brief Set the H flag.
   * \param h value
   */
  void SetFlagH(bool h);

  /**
   * \brief Get the L flag.
   * \return L flag
   */
  bool GetFlagL() const;
  
  /**
   * \brief Set the L flag.
   * \param l value
   */
  void SetFlagL(bool l);

  /**
   * \brief Get the K flag.
   * \return K flag
   */
  bool GetFlagK() const;
  
  /**
   * \brief Set the K flag.
   * \param k value
   */
  void SetFlagK(bool k);

  /**
   * \brief Get the M flag.
   * \return M flag
   */
  bool GetFlagM() const;
  
  /**
   * \brief Set the M flag.
   * \param m value
   */
  void SetFlagM(bool m);

  /**
   * \brief Get the R flag.
   * \return R flag
   */
  bool GetFlagR() const;
  
  /**
   * \brief Set the R flag.
   * \param r value
   */
  void SetFlagR(bool r);

  /**
   * \brief Get the P flag.
   * \return P flag
   */
  bool GetFlagP() const;
  
  /**
   * \brief Set the P flag.
   * \param p value
   */
  void SetFlagP(bool p);
  bool GetFlagT() const;
  void SetFlagT(bool t);

  /**
   * \brief Get the Reserved value.
   * \return Reserved value
   */
  uint16_t GetReserved2() const;
  
  /**
   * \brief Set the Reserved.
   * \param reserved reserved value
   */
  void SetReserved2(uint16_t reserved2);

  /**
   * \brief Get the Lifetime value.
   * \return Lifetime
   */
  uint16_t GetLifetime() const;
  
  /**
   * \brief Set the Lifetime.
   * \param lifetime lifetime value
   */
  void SetLifetime(uint16_t lifetime);

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

private:

  /**
   * \brief The Sequence field
   */
  uint16_t m_sequence;

  /**
   * \brief The A flag.
   */
  bool m_flagA;

  /**
   * \brief The H flag.
   */
  bool m_flagH;

  /**
   * \brief The L flag.
   */
  bool m_flagL;

  /**
   * \brief The K flag.
   */
  bool m_flagK;

  /**
   * \brief The M flag.
   */
  bool m_flagM;

  /**
   * \brief The R flag.
   */
  bool m_flagR;

  /**
   * \brief The P flag.
   */
  bool m_flagP;
  bool m_flagT;
  /**
   * \brief The reserved value.
   */
  uint16_t m_reserved2;

  /**
   * \brief The Lifetime.
   */
  uint16_t m_lifetime;
};

/**
 * \class Ipv6MobilityBindingAckHeader
 * \brief Ipv6 Mobility Binding Acknowledgement header.
 */
class Ipv6MobilityBindingAckHeader : public Ipv6MobilityHeader, public MobilityOptionField
{
public:
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
  Ipv6MobilityBindingAckHeader ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityBindingAckHeader ();

  /**
   * \brief Get the Status field.
   * \return status value
   */
  uint8_t GetStatus () const;

  /**
   * \brief Set the status field.
   * \param status the status value
   */
  void SetStatus (uint8_t status);
  
  /**
   * \brief Get the K flag.
   * \return K flag
   */
  bool GetFlagK() const;
  
  /**
   * \brief Set the K flag.
   * \param k value
   */
  void SetFlagK(bool k);

  /**
   * \brief Get the R flag.
   * \return R flag
   */
  bool GetFlagR() const;
  
  /**
   * \brief Set the R flag.
   * \param r value
   */
  void SetFlagR(bool r);

  /**
   * \brief Get the P flag.
   * \return P flag
   */
  bool GetFlagP() const;

  /**
   * \brief Set the P flag.
   * \param p value
   */
  void SetFlagP(bool p);
  bool GetFlagT() const;
  void SetFlagT(bool t);
  /**
   * \brief Get the Reserved2 field.
   * \return reserved2 value
   */
  uint8_t GetReserved2 () const;

  /**
   * \brief Set the reserved2 field.
   * \param reserved2 the reserved2 value
   */
  void SetReserved2 (uint8_t reserved2);
  
  /**
   * \brief Get the Sequence field.
   * \return sequence value
   */
  uint16_t GetSequence () const;

  /**
   * \brief Set the sequence field.
   * \param sequence the sequence value
   */
  void SetSequence (uint16_t sequence);

  /**
   * \brief Get the Lifetime value.
   * \return Lifetime
   */
  uint16_t GetLifetime() const;
  
  /**
   * \brief Set the Lifetime.
   * \param lifetime lifetime value
   */
  void SetLifetime(uint16_t lifetime);

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

private:

  /**
   * \brief The status value.
   */
  uint8_t m_status;

  /**
   * \brief The K flag.
   */
  bool m_flagK;

  /**
   * \brief The R flag.
   */
  bool m_flagR;

  /**
   * \brief The P flag.
   */
  /** G8XSU EDIT **/
  bool m_flagT;
  /**           **/
    /**
       * \brief The P flag.
       */

  bool m_flagP;
  /**
    * \brief The reserved value.
    */

  uint8_t m_reserved2;

  /**
   * \brief The Sequence field
   */
  uint16_t m_sequence;

  /**
   * \brief The Lifetime.
   */
  uint16_t m_lifetime;
};

} /* namespace ns3 */

#endif /* IPV6_MOBILITY_HEADER_H */
