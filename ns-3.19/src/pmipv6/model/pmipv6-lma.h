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

#ifndef PMIPV6_LMA_H
#define PMIPV6_LMA_H

#include "pmipv6-agent.h"
#include "binding-cache.h"

namespace ns3
{
class Packet;
class Ipv6MobilityOptionBundle;
class Pmipv6PrefixPool;

class Pmipv6Lma : public Pmipv6Agent {
public:
  Pmipv6Lma ();
  
  virtual ~Pmipv6Lma ();
  
  Ptr<Pmipv6PrefixPool> GetPrefixPool () const;
  void SetPrefixPool (Ptr<Pmipv6PrefixPool> pool);
  
  void DoDelayedRegistration (BindingCache::Entry *bce);
  
protected:
  virtual void DoDispose ();
  virtual void NotifyNewAggregate ();
  
  Ptr<Packet> BuildPba (BindingCache::Entry *bce, uint8_t status);
  Ptr<Packet> BuildPba (Ipv6MobilityBindingUpdateHeader pbu, Ipv6MobilityOptionBundle bundle, uint8_t status);
  Ptr<Packet> BuildHur (BindingCache::Entry *bce_new,BindingCache::Entry *bce_old, uint8_t status);
  virtual uint8_t HandlePbu (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  virtual uint8_t HandleHua (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  
  bool SetupTunnelAndRouting (BindingCache::Entry *bce);
  bool ModifyTunnelAndRouting (BindingCache::Entry *bce);
  void ClearTunnelAndRouting (BindingCache::Entry *bce); 

private:
  Ptr<BindingCache> m_bCache;
  
  Ptr<Pmipv6PrefixPool> m_prefixPool;
};

} /* namespace ns3 */

#endif /* PMIPV6_LMA_H */

