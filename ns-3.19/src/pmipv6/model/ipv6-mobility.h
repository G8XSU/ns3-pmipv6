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

#ifndef IPV6_MOBILITY_H
#define IPV6_MOBILITY_H

#include <list>

#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv6-interface.h"

namespace ns3
{

class Ipv6MobilityOptionBundle;

/**
 * \class Ipv6Mobility
 * \brief Ipv6 Mobility base
 *
 * If you want to implement a new Ipv6 Mobility, all you have to do is
 * implement a subclass of this class and add it to an Ipv6MobilityDemux.
 */
class Ipv6Mobility : public Object
{
public:
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6Mobility ();
  
  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);
  
  Ptr<Node> GetNode() const;
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityNumber () const = 0;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param offset the offset of the extension to process
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> p, Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface) = 0;
  
  virtual uint8_t ProcessOptions (Ptr<Packet> packet, uint8_t offset, uint8_t length, Ipv6MobilityOptionBundle &bundle);

protected:
  virtual void DoDispose ();

private:
  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
};

/**
 * \class Ipv6MobilityBindingUpdate
 * \brief Ipv6 Mobility Binding Update
 *
 * If you want to implement a new Ipv6 Mobility, all you have to do is
 * implement a subclass of this class and add it to an Ipv6MobilityDemux.
 */
class Ipv6MobilityBindingUpdate : public Ipv6Mobility
{
public:
  static const uint8_t MOB_NUMBER = 5;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityBindingUpdate ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param offset the offset of the extension to process
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> p, Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface);
  
private:

};

/**
 * \class Ipv6MobilityBindingAck
 * \brief Ipv6 Mobility Binding Acknowledgement
 *
 * If you want to implement a new Ipv6 Mobility, all you have to do is
 * implement a subclass of this class and add it to an Ipv6MobilityDemux.
 */
class Ipv6MobilityBindingAck : public Ipv6Mobility
{
public:
  static const uint8_t MOB_NUMBER = 6;

  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityBindingAck ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param offset the offset of the extension to process
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> p, Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface);
  
private:

};

} /* namespace ns3 */

#endif /* IPV6_MOBILITY_H */
