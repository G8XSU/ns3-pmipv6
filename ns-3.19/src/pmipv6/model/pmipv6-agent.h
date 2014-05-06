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

#ifndef PMIPV6_AGENT_H
#define PMIPV6_AGENT_H

#include "ns3/object.h"
#include "ns3/ipv6-address.h"

namespace ns3
{

class Node;
class Packet;
class Ipv6Interface;
class Pmipv6Profile;

/**
 * \class Pmipv6Agent
 * \brief An implementation of the PMIPv6 protocol.
 */
class Pmipv6Agent : public Object
{
public:
  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief Constructor.
   */
  Pmipv6Agent ();

  /**
   * \brief Destructor.
   */
  virtual ~Pmipv6Agent ();

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
  
  Ptr<Pmipv6Profile> GetProfile () const;
  void SetProfile (Ptr<Pmipv6Profile> pf);
  
  virtual uint8_t Receive (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  
  void SendMessage (Ptr<Packet> packet, Ipv6Address dst, uint32_t ttl);
  
protected:
  virtual uint8_t HandlePbu (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  virtual uint8_t HandlePba (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  virtual uint8_t HandleHur (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  virtual uint8_t HandleHua (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();
  
private:

  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
  
  Ptr<Pmipv6Profile> m_profile;
};

} /* namespace ns3 */

#endif /* PMIPV6_AGENT_H */

