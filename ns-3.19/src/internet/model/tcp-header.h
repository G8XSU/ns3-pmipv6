/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Georgia Tech Research Corporation
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
 * Author: Raj Bhattacharjea <raj.b@gatech.edu>
 */

#ifndef TCP_HEADER_H
#define TCP_HEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/sequence-number.h"

namespace ns3 {

/**
 * \ingroup tcp
 * \brief Header for the Transmission Control Protocol
 *
 * This class has fields corresponding to those in a network TCP header
 * (port numbers, sequence and acknowledgement numbers, flags, etc) as well
 * as methods for serialization to and deserialization from a byte buffer.
 */

class TcpHeader : public Header 
{
public:
  TcpHeader ();
  virtual ~TcpHeader ();

  /**
   * \brief Enable checksum calculation for TCP
   *
   * \todo currently has no effect
   */
  void EnableChecksums (void);
//Setters
/**
 * \param port The source port for this TcpHeader
 */
  void SetSourcePort (uint16_t port);
  /**
   * \param port the destination port for this TcpHeader
   */
  void SetDestinationPort (uint16_t port);
  /**
   * \param sequenceNumber the sequence number for this TcpHeader
   */
  void SetSequenceNumber (SequenceNumber32 sequenceNumber);
  /**
   * \param ackNumber the ACK number for this TcpHeader
   */
  void SetAckNumber (SequenceNumber32 ackNumber);
  /**
   * \param length the length of this TcpHeader
   */
  void SetLength (uint8_t length);
  /**
   * \param flags the flags for this TcpHeader
   */
  void SetFlags (uint8_t flags);
  /**
   * \param windowSize the window size for this TcpHeader
   */
  void SetWindowSize (uint16_t windowSize);
  /**
   * \param urgentPointer the urgent pointer for this TcpHeader
   */
  void SetUrgentPointer (uint16_t urgentPointer);


//Getters
/**
 * \return The source port for this TcpHeader
 */
  uint16_t GetSourcePort () const;
  /**
   * \return the destination port for this TcpHeader
   */
  uint16_t GetDestinationPort () const;
  /**
   * \return the sequence number for this TcpHeader
   */
  SequenceNumber32 GetSequenceNumber () const;
  /**
   * \return the ACK number for this TcpHeader
   */
  SequenceNumber32 GetAckNumber () const;
  /**
   * \return the length of this TcpHeader
   */
  uint8_t  GetLength () const;
  /**
   * \return the flags for this TcpHeader
   */
  uint8_t  GetFlags () const;
  /**
   * \return the window size for this TcpHeader
   */
  uint16_t GetWindowSize () const;
  /**
   * \return the urgent pointer for this TcpHeader
   */
  uint16_t GetUrgentPointer () const;

  /**
   * \brief Initialize the TCP checksum.
   *
   * If you want to use tcp checksums, you should call this
   * method prior to adding the header to a packet.
   *
   * \param source the IP source to use in the underlying
   *        IP packet.
   * \param destination the IP destination to use in the
   *        underlying IP packet.
   * \param protocol the protocol number to use in the underlying
   *        IP packet.
   *
   */
  void InitializeChecksum (Ipv4Address source, 
                           Ipv4Address destination,
                           uint8_t protocol);

  /**
   * \brief Initialize the TCP checksum.
   *
   * If you want to use tcp checksums, you should call this
   * method prior to adding the header to a packet.
   *
   * \param source the IP source to use in the underlying
   *        IP packet.
   * \param destination the IP destination to use in the
   *        underlying IP packet.
   * \param protocol the protocol number to use in the underlying
   *        IP packet.
   *
   */
  void InitializeChecksum (Ipv6Address source, 
                           Ipv6Address destination,
                           uint8_t protocol);

  /**
   * \brief Initialize the TCP checksum.
   *
   * If you want to use tcp checksums, you should call this
   * method prior to adding the header to a packet.
   *
   * \param source the IP source to use in the underlying
   *        IP packet.
   * \param destination the IP destination to use in the
   *        underlying IP packet.
   * \param protocol the protocol number to use in the underlying
   *        IP packet.
   *
   */
  void InitializeChecksum (Address source, 
                           Address destination,
                           uint8_t protocol);

  /**
   * \brief TCP flag field values
   */
  typedef enum { NONE = 0, FIN = 1, SYN = 2, RST = 4, PSH = 8, ACK = 16, 
                 URG = 32, ECE = 64, CWR = 128} Flags_t;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * \brief Is the TCP checksum correct ?
   * \returns true if the checksum is correct, false otherwise.
   */
  bool IsChecksumOk (void) const;

private:
  /**
   * \brief Calculate the header checksum
   * \param size packet size
   * \returns the checksum
   */
  uint16_t CalculateHeaderChecksum (uint16_t size) const;
  uint16_t m_sourcePort;        //!< Source port
  uint16_t m_destinationPort;   //!< Destination port
  SequenceNumber32 m_sequenceNumber;  //!< Sequence number
  SequenceNumber32 m_ackNumber;       //!< ACK number
  uint8_t m_length;             //!< Length (really a uint4_t)
  uint8_t m_flags;              //!< Flags (really a uint6_t)
  uint16_t m_windowSize;        //!< Window size
  uint16_t m_urgentPointer;     //!< Urgent pointer

  Address m_source;       //!< Source IP address
  Address m_destination;  //!< Destination IP address
  uint8_t m_protocol;     //!< Protocol number

  bool m_calcChecksum;    //!< Flag to calculate checksum
  bool m_goodChecksum;    //!< Flag to indicate that checksum is correct
};

} // namespace ns3

#endif /* TCP_HEADER */
