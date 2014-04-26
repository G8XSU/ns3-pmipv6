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

#ifndef IPV6_STATIC_SOURCE_ROUTING_HELPER_H
#define IPV6_STATIC_SOURCE_ROUTING_HELPER_H

#include "ns3/ipv6.h"
#include "ns3/ptr.h"
#include "ns3/ipv6-address.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/ipv6-routing-helper.h"

#include "ns3/ipv6-static-source-routing.h"

namespace ns3 {

/**
 * \brief Helper class that adds ns3::Ipv6StaticSourceRouting objects
 *
 * This class is expected to be used in conjunction with 
 * ns3::InternetStackHelper::SetRoutingHelper
 */
class Ipv6StaticSourceRoutingHelper : public Ipv6RoutingHelper
{
public:
  /**
   * \brief Constructor.
   */
  Ipv6StaticSourceRoutingHelper ();

  /**
   * \brief Construct an Ipv6ListRoutingHelper from another previously 
   * initialized instance (Copy Constructor).
   */
  Ipv6StaticSourceRoutingHelper (const Ipv6StaticSourceRoutingHelper &);

  /**
   * \internal
   * \returns pointer to clone of this Ipv6StaticSourceRoutingHelper
   *
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  Ipv6StaticSourceRoutingHelper* Copy (void) const;

  /**
   * \param node the node on which the routing protocol will run
   * \returns a newly-created routing protocol
   *
   * This method will be called by ns3::InternetStackHelper::Install
   */
  virtual Ptr<Ipv6RoutingProtocol> Create (Ptr<Node> node) const;

  /**
   * \brief Get Ipv6StaticRouting pointer from IPv6 stack.
   * \param ipv6 Ipv6 pointer
   * \return Ipv6StaticRouting pointer or 0 if not exist
   */
  Ptr<Ipv6StaticSourceRouting> GetStaticSourceRouting (Ptr<Ipv6> ipv6) const;

private:
  /**
   * \internal
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  Ipv6StaticSourceRoutingHelper &operator = (const Ipv6StaticSourceRoutingHelper &o);
};

} // namespace ns3

#endif /* IPV6_STATIC_SOURCE_ROUTING_HELPER_H */

