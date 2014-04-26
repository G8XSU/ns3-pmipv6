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

#ifndef IPV6_MOBILITY_L4_PROTOCOL_H
#define IPV6_MOBILITY_L4_PROTOCOL_H

#include "ns3/ipv6-address.h"
#include "ns3/ip-l4-protocol.h"

namespace ns3
{

class Node;
class Packet;

/**
 * \class Ipv6MobilityL4Protocol
 * \brief An implementation of the Mobile Ipv6 protocol.
 */
class Ipv6MobilityL4Protocol : public IpL4Protocol
{
public:
  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief IPv6 Mobility protocol number (135).
   */
  static const uint8_t PROT_NUMBER;
  
  /**
   * \brief Binding Update lifetime (0x3fffc seconds)
   */
  static const double MAX_BINDING_LIFETIME;

  /**
   * \brief Initial Binding Ack Timeout for First Registration (1.5 seconds)
   */
  static const double INITIAL_BINDING_ACK_TIMEOUT_FIRSTREG;

  /**
   * \brief Initial Binding Ack Timeout for Re-Registration (1 second)
   */
  static const double INITIAL_BINDING_ACK_TIMEOUT_REREG;
  
  /**
   * \brief Binding Update Maximum retry count (=3)
   */  
  static const uint8_t MAX_BINDING_UPDATE_RETRY_COUNT;
  
  /**
   * \brief The amount of time in milliseconds the local mobility anchor before delete BCE entry (10000ms)
   */  
  static const uint32_t MIN_DELAY_BEFORE_BCE_DELETE;
  
  /**
   * \brief The amount of time the LMA MUST wait for the de-reg messsage (1500ms)
   */  
  static const uint32_t MIN_DELAY_BEFORE_NEW_BCE_ASSIGN;

  /**
   * \brief The maximum amount of time difference between timestamps
   */  
  static const uint32_t TIMESTAMP_VALIDITY_WINDOW;

  /**
   * \brief Get PMIPv6 protocol number.
   * \return protocol number
   */
  static uint16_t GetStaticProtocolNumber ();

  /**
   * \brief Constructor.
   */
  Ipv6MobilityL4Protocol ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityL4Protocol ();

  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);
  
  /**
   * \brief Get the node.
   * \return node
   */
  Ptr<Node> GetNode (void);

    /**
   * \brief This method is called by AddAgregate and completes the aggregation
   * by setting the node in the Ipv6 Mobility stack.
   */
  virtual void NotifyNewAggregate ();

  /**
   * \brief Get the protocol number.
   * \return protocol number
   */
  virtual int GetProtocolNumber () const;

  /**
   * \brief Get the version of the protocol.
   * \return version
   */
  virtual int GetVersion () const;

  /**
   * \brief Send a packet via IPv6 Mobility, note that packet already contains IPv6 Mobility header.
   * \param packet the packet to send which contains IPv6 Mobility header
   * \param src source address
   * \param dst destination address
   * \param ttl next hop limit
   */
  void SendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl);
  
  /**
   * \brief Receive method.
   * \param p the packet
   * \param header ipv6 header
   * \param interface the interface from which the packet is coming
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p, Ipv6Header const &header, Ptr<Ipv6Interface> interface);

  /**
   * \brief Receive method.
   * \param p the packet
   * \param header ipv4 header
   * \param interface the interface from which the packet is coming
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p, Ipv4Header const &header, Ptr<Ipv4Interface> incomingInterface);

  virtual void SetDownTarget (IpL4Protocol::DownTargetCallback cb);
  virtual void SetDownTarget6 (IpL4Protocol::DownTargetCallback6 cb);
  virtual IpL4Protocol::DownTargetCallback GetDownTarget (void) const;
  virtual IpL4Protocol::DownTargetCallback6 GetDownTarget6 (void) const;

  /**
   * \brief Register the IPv6 Mobility.
   */
  virtual void RegisterMobility();

  /**
   * \brief Register the IPv6 Mobility Options.
   */
  virtual void RegisterMobilityOptions ();

protected:
 
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();
  
private:

  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
  
};

} /* namespace ns3 */

#endif /* IPV6_MOBILITY_L4_PROTOCOL_H */
