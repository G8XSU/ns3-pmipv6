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

#ifndef PMIPV6_PREFIX_POOL_H
#define PMIPV6_PREFIX_POOL_H

#include "ns3/simple-ref-count.h"
#include "ns3/ipv6-address.h"

namespace ns3
{

class Pmipv6PrefixPool : public SimpleRefCount<Pmipv6PrefixPool>
{
public:
  Pmipv6PrefixPool(Ipv6Address prefixBegin, uint8_t prefixLen);
  
  Ipv6Address Assign();
protected:

private:
  Ipv6Address m_prefixBegin;
  uint8_t m_prefixBeginLen;
  
  uint64_t m_lastPrefixIndex;
};

} /* namespace ns3 */

#endif /* PMIPV6_PREFIX_POOL_H */
