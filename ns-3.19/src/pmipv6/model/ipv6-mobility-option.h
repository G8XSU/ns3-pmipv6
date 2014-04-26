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

#ifndef IPV6_MOBILITY_OPTION_H
#define IPV6_MOBILITY_OPTION_H

#include <list>

#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/identifier.h"

namespace ns3
{

class Ipv6MobilityOptionBundle;

/**
 * \class Ipv6MobilityOption
 * \brief Ipv6 Mobility Option base
 *
 * If you want to implement a new Ipv6 Mobility option, all you have to do is
 * implement a subclass of this class and add it to an Ipv6MobilityOptionDemux.
 */
class Ipv6MobilityOption : public Object
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
  virtual ~Ipv6MobilityOption ();
  
  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const = 0;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle) = 0;

protected:
  virtual void DoDispose ();

private:
  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
};

class Ipv6MobilityOptionBundle
{
public:
  Ipv6MobilityOptionBundle();
  
  Identifier GetMnIdentifier() const;
  void SetMnIdentifier(Identifier mnId);
  
  Identifier GetMnLinkIdentifier() const;
  void SetMnLinkIdentifier(Identifier mnLinkId);
  
  std::list<Ipv6Address> GetHomeNetworkPrefixes() const;
  void SetHomeNetworkPrefixes(std::list<Ipv6Address> hnps);
  void AddHomeNetworkPrefix(Ipv6Address addr);
  
  Ipv6Address GetMagLinkAddress() const;
  void SetMagLinkAddress(Ipv6Address lla);
  
  uint8_t GetAccessTechnologyType() const;
  void SetAccessTechnologyType(uint8_t att);
  
  uint8_t GetHandoffIndicator() const;
  void SetHandoffIndicator(uint8_t hi);
  
  Time GetTimestamp() const;
  void SetTimestamp(Time tm);
  
protected:
private:
  //for PMIPv6
  Identifier m_mnIdentifier;
  Identifier m_mnLinkIdentifier;
  std::list<Ipv6Address> m_homeNetworkPrefixes;
  Ipv6Address m_magLinkAddress;
  uint8_t m_accessTechnologyType;
  uint8_t m_handoffIndicator;
  Time m_timestamp;
};

/**
 * \class Ipv6MobilityOptionPad1
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionPad1 : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 0;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionPad1 ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionPadn
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionPadn : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 1;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionPadn ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionMobileNodeIdentifier
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionMobileNodeIdentifier : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 8;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionMobileNodeIdentifier ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionHomeNetworkPrefix
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionHomeNetworkPrefix : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 22;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionHomeNetworkPrefix ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionHandoffIndicator
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionHandoffIndicator : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 23;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionHandoffIndicator ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionAccessTechnologyType
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionAccessTechnologyType : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 24;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionAccessTechnologyType ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionMobileNodeLinkLayerIdentifier
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionMobileNodeLinkLayerIdentifier : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 25;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionMobileNodeLinkLayerIdentifier ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionLinkLocalAddress
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionLinkLocalAddress : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 26;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionLinkLocalAddress ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionTimestamp
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionTimestamp : public Ipv6MobilityOption
{
public:
  static const uint8_t OPT_NUMBER = 27;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionTimestamp ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, Ipv6MobilityOptionBundle& bundle);
  
private:
};

} /* namespace ns3 */

#endif /* IPV6_MOBILITY_OPTION_H */
