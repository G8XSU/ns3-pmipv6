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

#ifndef IPV6_MOBILITY_DEMUX_H
#define IPV6_MOBILITY_DEMUX_H

#include <list>

#include "ns3/header.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

namespace ns3
{

class Ipv6Mobility;
class Node;

/**
 * \class Ipv6MobilityDemux
 * \brief Ipv6 Mobility Demux.
 */
class Ipv6MobilityDemux : public Object
{
public:
  /**
   * \brief The interface ID.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor.
   */
  Ipv6MobilityDemux ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityDemux ();

  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);

  /**
   * \brief Insert a new IPv6 Mobility.
   * \param option the option to insert
   */
  void Insert (Ptr<Ipv6Mobility> mobility);

  /**
   * \brief Get the mobility corresponding to mobilityNumber.
   * \param mobilityNumber the mobility number of the option to retrieve
   * \return a matching IPv6 Mobility mobility
   */
  Ptr<Ipv6Mobility> GetMobility (int mobilityNumber);

  /**
   * \brief Remove an mobility from this demux.
   * \param mobility pointer on the mobility to remove
   */
  void Remove (Ptr<Ipv6Mobility> mobility);

protected:
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose();

private:
  typedef std::list<Ptr<Ipv6Mobility> > Ipv6MobilityList_t;

  /**
   * \brief List of IPv6 Mobility supported.
   */
  Ipv6MobilityList_t m_mobilities;

  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
};

} /* namespace ns3 */

#endif /* IPV6_MOBILITY_DEMUX_H */
