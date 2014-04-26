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

#include "unicast-radvd-interface.h"

namespace ns3 
{

uint32_t UnicastRadvdInterface::m_idGen = 1;

UnicastRadvdInterface::UnicastRadvdInterface(uint32_t interface, enum UnicastRadvdInterface::InterfaceType interfaceType)
 : RadvdInterface(interface),
   m_interfaceType (interfaceType),
   m_id (m_idGen++)
{

}

UnicastRadvdInterface::UnicastRadvdInterface(uint32_t interface, uint32_t maxRtrAdvInterval, uint32_t minRtrAdvInterval, enum UnicastRadvdInterface::InterfaceType interfaceType)
 : RadvdInterface(interface, maxRtrAdvInterval, minRtrAdvInterval),
   m_interfaceType (interfaceType),
   m_id (m_idGen++)
{
  
}

enum UnicastRadvdInterface::InterfaceType UnicastRadvdInterface::GetInterfaceType ()
{
  return m_interfaceType;
}

uint32_t UnicastRadvdInterface::GetId () const
{
  return m_id;
}

Address UnicastRadvdInterface::GetPhysicalAddress () const
{
  return m_physicalAddress;
}

void UnicastRadvdInterface::SetPhysicalAddress (Address addr)
{
  m_physicalAddress = addr;
}

uint32_t UnicastRadvdInterface::GetTunnelId ()
{
  return m_teid;
}

void UnicastRadvdInterface::SetTunnelId (uint32_t teid)
{
  m_teid = teid;
}

uint64_t UnicastRadvdInterface::GetImsi ()
{
  return m_imsi;
}

void UnicastRadvdInterface::SetImsi (uint64_t imsi)
{
  m_imsi = imsi;
}

}
