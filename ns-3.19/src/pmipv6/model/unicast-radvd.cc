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
 
#include "ns3/log.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-header.h"
#include "ns3/icmpv6-header.h"
#include "ns3/packet-socket-address.h"

#include "unicast-radvd.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("UnicastRadvd");

NS_OBJECT_ENSURE_REGISTERED (UnicastRadvd);

TypeId UnicastRadvd::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::UnicastRadvd")
    .SetParent<Application> ()
    ;
  return tid;
}

void UnicastRadvd::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  for (RadvdInterfaceListI it = m_configurations.begin () ; it != m_configurations.end () ; ++it)
    {
      *it = 0;
    }
  m_configurations.clear ();
  Application::DoDispose ();
}

void UnicastRadvd::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

//  for (RadvdInterfaceListCI it = m_configurations.begin () ; it != m_configurations.end () ; it++)
//    {
//      m_eventIds[(*it)->GetId ()] = EventId ();
//      ScheduleTransmit (Seconds (0.), (*it), m_eventIds[(*it)->GetId ()], Ipv6Address::GetAllNodesMulticast (), true);
//    }
}

void UnicastRadvd::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
//  for (EventIdMapI it = m_eventIds.begin () ; it != m_eventIds.end () ; ++it)
//    {
//      Simulator::Cancel ((*it).second);
//    }
//  m_eventIds.clear ();
}

void UnicastRadvd::AddConfiguration (Ptr<UnicastRadvdInterface> routerInterface)
{
  NS_LOG_FUNCTION ( this << routerInterface );
  
  m_configurations.push_back (routerInterface);
  if (IsAppStarted ())
    {
      NS_LOG_LOGIC ("Application is already started. Adding and Scheduling.");
      if (m_eventIds.find(routerInterface->GetId()) != m_eventIds.end() )
        {
          m_eventIds[routerInterface->GetId()].Cancel();
        }
      m_eventIds[routerInterface->GetId()] = EventId();
      ScheduleTransmit (Seconds (0.), routerInterface, m_eventIds[routerInterface->GetId ()], Ipv6Address::GetAllNodesMulticast (), true); 
    }
}

void UnicastRadvd::RemoveConfiguration (Ptr<UnicastRadvdInterface> routerInterface)
{
  NS_LOG_FUNCTION ( this << routerInterface );
  
  if ( m_eventIds.find(routerInterface->GetId()) != m_eventIds.end() )
    {
      m_eventIds[routerInterface->GetId()].Cancel();
    }
  m_configurations.remove (routerInterface);
}

void UnicastRadvd::RemoveConfiguration (int32_t ifIndex)
{
  NS_LOG_FUNCTION ( this << ifIndex );
  
  for ( RadvdInterfaceListI i = m_configurations.begin(); i != m_configurations.end(); i++ )
    {
      if((*i)->GetId() == (uint32_t)ifIndex)
        {
          RemoveConfiguration ((*i));
          return;
        }
    }
}
  
void UnicastRadvd::ScheduleTransmit (Time dt, Ptr<UnicastRadvdInterface> config, EventId& eventId, Ipv6Address dst, bool reschedule)
{
  NS_LOG_FUNCTION (this << dt);
  eventId = Simulator::Schedule (dt, &UnicastRadvd::Send, this, config, dst, reschedule);
}

} /* namespace ns3 */

