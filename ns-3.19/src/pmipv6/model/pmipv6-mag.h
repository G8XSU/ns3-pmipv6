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

#ifndef PMIPV6_MAG_H
#define PMIPV6_MAG_H

#include "pmipv6-agent.h"
#include "binding-update-list.h"

namespace ns3
{
class UnicastRadvd;
class Mac48Address;

class Pmipv6Mag : public Pmipv6Agent
{
public:
  Pmipv6Mag();
  virtual ~Pmipv6Mag();
  
  bool IsUseRemoteAP() const;
  void UseRemoteAP(bool remoteAp);
  bool IsLteMag () const;
  void SetLteMag (bool isLteMag);

  uint16_t GetSequence();
  
  bool SetupTunnelAndRouting(BindingUpdateList::Entry *bule);
  void ClearTunnelAndRouting(BindingUpdateList::Entry *bule); 

  void SetupRegularRadvdInterface(BindingUpdateList::Entry *bule);
  void SetupLteRadvdInterface(BindingUpdateList::Entry *bule);
  void ClearRadvdInterface(BindingUpdateList::Entry *bule);
  
  Ptr<Packet> BuildPbu(BindingUpdateList::Entry *bule);
  Ptr<Packet> BuildHua(BindingUpdateList::Entry *bule,std::list<Ipv6Address> new_hnps);
  
protected:
  virtual void NotifyNewAggregate();
  
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();

  Ipv6Address GetLinkLocalAddress(Ipv6Address addr);
  
  Ptr<UnicastRadvd> GetRadvd() const;
  
  virtual void HandleRegularNewNode (Mac48Address from, Mac48Address to, uint8_t att);
  virtual void HandleLteNewNode (uint32_t teid, uint64_t imsi, uint8_t att);
  virtual uint8_t HandlePba(Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
  virtual uint8_t HandleHur(Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface);
private:
  
  bool m_useRemoteAp;
  bool m_isLteMag;
  
  Callback<void, uint64_t, Ipv6Address> m_setIp;

  uint16_t m_sequence;
  
  Ptr<BindingUpdateList> m_buList;
  
  Ptr<UnicastRadvd> m_radvd;

  // Interface index where MAG is on LTE.
  int16_t m_ifIndex;
};

} /* namespace ns3 */

#endif /* PMIPV6_MAG_H */

