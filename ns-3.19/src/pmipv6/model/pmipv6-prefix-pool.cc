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
#include "pmipv6-prefix-pool.h"

NS_LOG_COMPONENT_DEFINE ("Pmipv6PrefixPool");
 
namespace ns3
{
 
Pmipv6PrefixPool::Pmipv6PrefixPool (Ipv6Address prefixBegin, uint8_t prefixLen)
: m_prefixBegin (prefixBegin),
  m_prefixBeginLen (prefixLen),
  m_lastPrefixIndex (0)
{
 NS_LOG_FUNCTION (this << prefixBegin << (uint32_t) prefixLen);

 NS_ASSERT (prefixLen < 64);
}

Ipv6Address Pmipv6PrefixPool::Assign ()
{
 NS_LOG_FUNCTION_NOARGS ();

 uint8_t buf[16];
 Ipv6Address addr;
 int len;

 m_prefixBegin.Serialize (buf);
 len = 8 - m_prefixBeginLen / 8;
 m_lastPrefixIndex++;
 
 for (int i = 0; i < len; i++)
   {
     buf[7-i] = (uint8_t)((m_lastPrefixIndex >> (i * 8)) & 0xff);
   }
 addr.Set(buf);
 return addr;
}
 
}
