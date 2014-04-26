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

#ifndef IPV6_MOBILITY_OPTION_DEMUX_H
#define IPV6_MOBILITY_OPTION_DEMUX_H

#include <list>

#include "ns3/header.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

namespace ns3
{

class Ipv6MobilityOption;
class Node;

/**
 * \class Ipv6MobilityOptionDemux
 * \brief Ipv6 Mobility Option Demux.
 */
class Ipv6MobilityOptionDemux : public Object
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
  Ipv6MobilityOptionDemux ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionDemux ();

  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);

  /**
   * \brief Insert a new IPv6 Mobility Option.
   * \param option the option to insert
   */
  void Insert (Ptr<Ipv6MobilityOption> option);

  /**
   * \brief Get the option corresponding to optionNumber.
   * \param optionNumber the option number of the option to retrieve
   * \return a matching IPv6 Mobility option
   */
  Ptr<Ipv6MobilityOption> GetOption (int optionNumber);

  /**
   * \brief Remove an option from this demux.
   * \param option pointer on the option to remove
   */
  void Remove (Ptr<Ipv6MobilityOption> option);

protected:
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose();

private:
  typedef std::list<Ptr<Ipv6MobilityOption> > Ipv6MobilityOptionList_t;

  /**
   * \brief List of IPv6 Options supported.
   */
  Ipv6MobilityOptionList_t m_options;

  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
};

} /* namespace ns3 */

#endif /* IPV6_MOBILITY_OPTION_DEMUX_H */
