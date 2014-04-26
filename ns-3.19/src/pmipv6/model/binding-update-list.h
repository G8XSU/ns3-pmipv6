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

#ifndef BINDING_UPDATE_LIST_H
#define BINDING_UPDATE_LIST_H

#include <stdint.h>

#include <list>

#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/net-device.h"
#include "ns3/ipv6-address.h"
#include "ns3/ptr.h"
#include "ns3/timer.h"
#include "ns3/sgi-hashmap.h"

#include "identifier.h"

namespace ns3
{

class BindingUpdateList : public Object
{
public:
  class Entry;
  
  static TypeId GetTypeId ();

  BindingUpdateList();
  
  ~BindingUpdateList();
  
  BindingUpdateList::Entry *Lookup(Identifier mnId);
  
  BindingUpdateList::Entry *Add(Identifier mnId);
  
  void Remove(BindingUpdateList::Entry *entry);
  
  void Flush();
  
  Ptr<Node> GetNode() const;
  void SetNode(Ptr<Node> node);
  
  class Entry
  {
  public:
    Entry(Ptr<BindingUpdateList> bul);
	
    bool IsUnreachable() const;
    bool IsUpdating() const;
    bool IsRefreshing() const;
    bool IsReachable() const;

    void MarkUnreachable();
    void MarkUpdating();
    void MarkRefreshing();
    void MarkReachable();

    //timer processing
    void StartRetransTimer();
    void StopRetransTimer();

    void StartReachableTimer();
    void StopReachableTimer();

    void StartRefreshTimer();
    void StopRefreshTimer();

    void FunctionRetransTimeout();
    void FunctionReachableTimeout();
    void FunctionRefreshTimeout();

    bool Match(Identifier mnId) const;

    Identifier GetMnIdentifier() const;
    void SetMnIdentifier(Identifier mnId);

    Identifier GetMnLinkIdentifier() const;
    void SetMnLinkIdentifier(Identifier mnLinkId);

    std::list<Ipv6Address> GetHomeNetworkPrefixes() const;
    void SetHomeNetworkPrefixes(std::list<Ipv6Address> hnpList);

    Ipv6Address GetMagLinkAddress() const;
    void SetMagLinkAddress(Ipv6Address lla);

    Ipv6Address GetLmaAddress() const;
    void SetLmaAddress(Ipv6Address lmaa);

    int16_t GetIfIndex() const;
    void SetIfIndex(int16_t ifi);

    int16_t GetTunnelIfIndex() const;
    void SetTunnelIfIndex(int16_t tunnelif);

    uint8_t GetAccessTechnologyType() const;
    void SetAccessTechnologyType(uint8_t att);

    uint8_t GetHandoffIndicator() const;
    void SetHandoffIndicator(uint8_t hi);

    Time GetLastBindingUpdateTime() const;
    void SetLastBindingUpdateTime(Time tm);

    Time GetReachableTime() const;
    void SetReachableTime(Time tm);

    uint16_t GetLastBindingUpdateSequence() const;
    void SetLastBindingUpdateSequence(uint16_t seq);

    Ptr<Packet> GetPbuPacket() const;
    void SetPbuPacket(Ptr<Packet> pkt);

    uint8_t GetRetryCount() const;
    void IncreaseRetryCount();
    void ResetRetryCount();

    Entry *GetNext() const;
    void SetNext(Entry *entry);

    int32_t GetRadvdIfIndex() const;
    void SetRadvdIfIndex(int32_t ifIndex);

    int64_t GetImsi () const;
    void SetImsi (int64_t imsi);
	
    int32_t GetTunnelId () const;
    void SetTunnelId (int32_t teid);

  private:
    enum BindingUpdateState_e
	{
          UNREACHABLE,
	  UPDATING,
	  REFRESHING,
	  REACHABLE,
	};

    Ptr<BindingUpdateList> m_buList;
	
    BindingUpdateState_e m_state;
	
    Identifier m_mnIdentifier;
	
    Identifier m_mnLinkIdentifier;

    std::list<Ipv6Address> m_homeNetworkPrefixes;

    Ipv6Address m_magLinkAddress;

    Ipv6Address m_lmaAddress;

    int16_t m_ifIndex;

    int16_t m_tunnelIfIndex;

    uint8_t m_accessTechnologyType;

    uint8_t m_handoffIndicator;

    Time m_lastBindingUpdateTime;

    Time m_reachableTime;

    Timer m_retransTimer;

    Timer m_reachableTimer;

    Timer m_refreshTimer;

    uint16_t m_lastBindingUpdateSequence;
    Ptr<Packet> m_pktPbu;

    uint8_t m_retryCount;

    //internal
    int32_t m_radvdIfIndex; // Radvd Interface Index

    Entry *m_next;

    // internal members for LTE
    uint64_t m_imsi;
    uint32_t m_teid;
  };
  
protected:
  virtual void DoDispose();

private:
  typedef sgi::hash_map<Identifier, BindingUpdateList::Entry *, IdentifierHash> BUList;
  typedef sgi::hash_map<Identifier, BindingUpdateList::Entry *, IdentifierHash>::iterator BUListI;
  
  BUList m_buList;
  
  Ptr<Node> m_node;
};

} /* ns3 */

#endif /* BINDING_UPDATE_LIST_H */
