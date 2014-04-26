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

#ifndef UNICAST_RADVD_INTERFACE_H
#define UNICAST_RADVD_INTERFACE_H

#include "ns3/radvd-interface.h"

namespace ns3
{

/**
 * \ingroup radvd
 * \class UnicastRadvdInterface
 * \brief Unicast Radvd interface configuration.
 */
class UnicastRadvdInterface : public RadvdInterface
{
public:
  enum InterfaceType
  {
    REGULAR,
    LTE
  };

  UnicastRadvdInterface(uint32_t interface, enum InterfaceType interfaceType = REGULAR);
  
  UnicastRadvdInterface(uint32_t interface, uint32_t maxRtrAdvInterval, uint32_t minRtrAdvInterval, enum InterfaceType interfaceType = REGULAR);
  
  enum InterfaceType GetInterfaceType ();

  uint32_t GetId () const;

  Address GetPhysicalAddress() const;

  void SetPhysicalAddress(Address addr);

  uint32_t GetTunnelId ();

  void SetTunnelId (uint32_t teid);

  uint64_t GetImsi ();

  void SetImsi (uint64_t imsi);

private:
  enum InterfaceType m_interfaceType;
  Address m_physicalAddress;
  uint32_t m_teid;
  uint64_t m_imsi;

protected:
  uint32_t m_id;
  
  static uint32_t m_idGen;
};

} /* namespace ns3 */

#endif /* UNICAST_RADVD_INTERFACE_H */

