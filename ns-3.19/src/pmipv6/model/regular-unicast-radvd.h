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

#ifndef REGULAR_UNICAST_RADVD_H
#define REGULAR_UNICAST_RADVD_H

#include <map>

#include "ns3/application.h"
#include "ns3/socket.h"

#include "unicast-radvd.h"

namespace ns3
{

/**
 * \ingroup applications 
 * \defgroup unicastradvd UnicastRadvd
 */

/**
 * \ingroup unicast-radvd
 * \class RegularUnicastRadvd
 * \brief Router advertisement daemon with MAC unicast.
 */
class RegularUnicastRadvd : public UnicastRadvd
{
public:
  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor.
   */
  RegularUnicastRadvd ();

  /**
   * \brief Destructor.
   */
  virtual ~RegularUnicastRadvd ();

protected:
  /**
   * \brief Dispose the instance.
   */
  virtual void DoDispose ();

  /**
   * \brief Send a packet.
   * \param config interface configuration
   * \param dst destination address (default ff02::1)
   * \param reschedule if true another send will be reschedule (periodic)
   */
  virtual void Send (Ptr<UnicastRadvdInterface> config, Ipv6Address dst = Ipv6Address::GetAllNodesMulticast (), bool reschedule = false);

  virtual bool IsAppStarted ();

private:

  /**
   * \brief Start the application.
   */
  virtual void StartApplication ();

  /**
   * \brief Stop the application.
   */
  virtual void StopApplication ();

  /**
   * \brief Raw socket to send RA.
   */
  Ptr<Socket> m_socket;
};

} /* namespace ns3 */

#endif /* REGULAR_UNICAST_RADVD_H */

