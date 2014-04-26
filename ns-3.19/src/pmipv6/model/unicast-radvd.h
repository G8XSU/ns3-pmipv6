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

#ifndef UNICAST_RADVD_H
#define UNICAST_RADVD_H

#include <map>

#include "ns3/application.h"
#include "ns3/socket.h"

#include "unicast-radvd-interface.h"

namespace ns3
{

/**
 * \ingroup applications 
 * \defgroup unicastradvd UnicastRadvd
 */

/**
 * \ingroup unicast-radvd
 * \class UnicastRadvd
 * \brief Router advertisement daemon with MAC unicast.
 */
class UnicastRadvd : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Default value for maximum delay of RA (ms)
   */
  static const uint32_t MAX_RA_DELAY_TIME = 500;

  /**
   * \brief Add configuration for an interface;
   * \param routerInterface configuration
   */
  void AddConfiguration (Ptr<UnicastRadvdInterface> routerInterface);
  
  void RemoveConfiguration (Ptr<UnicastRadvdInterface> routerInterface);
  void RemoveConfiguration (int32_t ifIndex);

protected:
  typedef std::list<Ptr<UnicastRadvdInterface> > RadvdInterfaceList;
  typedef std::list<Ptr<UnicastRadvdInterface> >::iterator RadvdInterfaceListI;
  typedef std::list<Ptr<UnicastRadvdInterface> >::const_iterator RadvdInterfaceListCI;

  typedef std::map<uint32_t, EventId> EventIdMap;
  typedef std::map<uint32_t, EventId>::iterator EventIdMapI;
  typedef std::map<uint32_t, EventId>::const_iterator EventIdMapCI;
  /**
   * \brief Dispose the instance.
   */
  virtual void DoDispose ();

  /**
   * \brief Schedule sending a packet.
   * \param dt interval between packet
   * \param config interface configuration
   * \param eventId event ID associated
   * \param dst IPv6 destination address
   * \param reschedule if true another send will be reschedule (periodic)
   */
  void ScheduleTransmit (Time dt, Ptr<UnicastRadvdInterface> config, EventId& eventId, Ipv6Address dst = Ipv6Address::GetAllNodesMulticast (), bool reschedule = false);

  /**
   * \brief Send a packet.
   * \param config interface configuration
   * \param dst destination address (default ff02::1)
   * \param reschedule if true another send will be reschedule (periodic)
   */
  virtual void Send (Ptr<UnicastRadvdInterface> config, Ipv6Address dst = Ipv6Address::GetAllNodesMulticast (), bool reschedule = false) = 0;

  virtual bool IsAppStarted () = 0;

  /**
   * \brief List of configuration for interface.
   */
  RadvdInterfaceList m_configurations;

  /**
   * \brief Event ID map.
   */
  EventIdMap m_eventIds;

private:
  /**
   * \brief Start the application.
   */
  virtual void StartApplication ();

  /**
   * \brief Stop the application.
   */
  virtual void StopApplication ();

};

} /* namespace ns3 */

#endif /* UNICAST_RADVD_H */

