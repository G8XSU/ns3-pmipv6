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
#include "ns3/uinteger.h"
#include "ns3/node.h"

#include "binding-update-list.h"

#include "ipv6-mobility-l4-protocol.h"

#include "ipv6-mobility-header.h"
#include "ipv6-mobility.h"

#include "ipv6-mobility-option-header.h"
#include "ipv6-mobility-option.h"

#include "pmipv6-mag.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("BindingUpdateList");

TypeId BindingUpdateList::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BindingUpdateList")
    .SetParent<Object> ()
    ;
  return tid;
} 

BindingUpdateList::BindingUpdateList ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

BindingUpdateList::~BindingUpdateList ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Flush ();
}

void BindingUpdateList::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Flush ();
  Object::DoDispose ();
}

BindingUpdateList::Entry* BindingUpdateList::Lookup (Identifier mnId)
{
  NS_LOG_FUNCTION (this << mnId );
  
  if ( m_buList.find (mnId) != m_buList.end ())
    {
      BindingUpdateList::Entry* entry = m_buList[mnId];
      
      return entry;
    }
  return 0;
}

BindingUpdateList::Entry* BindingUpdateList::Add (Identifier mnId)
{
  NS_LOG_FUNCTION (this << mnId );
  if(Lookup(mnId) == 0)
	  NS_ASSERT( Lookup(mnId) == 0 );
  
  BindingUpdateList::Entry* entry = new BindingUpdateList::Entry (this);
  
  entry->SetMnIdentifier(mnId);
  
  if( m_buList.find (mnId) != m_buList.end ())
    {
      BindingUpdateList::Entry* entry2 = m_buList[mnId];
      
      entry->SetNext(entry2);
    }

  m_buList[mnId] = entry;
  
  return entry;
}

void BindingUpdateList::Remove (BindingUpdateList::Entry* entry)
{
  NS_LOG_FUNCTION_NOARGS ();

  for (BUListI i = m_buList.begin () ; i != m_buList.end () ; i++)
    {
      if ((*i).second == entry)
        {
          m_buList.erase (i);
          delete entry;
          return;
        }
    }
}

void BindingUpdateList::Flush ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (BUListI i = m_buList.begin () ; i != m_buList.end () ; i++)
    {
      delete (*i).second; /* delete the pointer BindingUpdateList::Entry */
    }

  m_buList.erase (m_buList.begin (), m_buList.end ());
}

Ptr<Node> BindingUpdateList::GetNode() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_node;
}

void BindingUpdateList::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION ( this << node );
  
  m_node = node;
}

BindingUpdateList::Entry::Entry (Ptr<BindingUpdateList> bul)
  : m_buList (bul),
  m_state (UNREACHABLE),
  m_ifIndex(-1),
  m_tunnelIfIndex(-1),
  m_retransTimer (Timer::CANCEL_ON_DESTROY),
  m_reachableTimer (Timer::CANCEL_ON_DESTROY),
  m_refreshTimer (Timer::CANCEL_ON_DESTROY),
  m_radvdIfIndex (-1),
  m_next (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void BindingUpdateList::Entry::FunctionRefreshTimeout ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Pmipv6Mag> mag = m_buList->GetNode ()->GetObject<Pmipv6Mag> ();
   
  if (mag == 0)
    {
      NS_LOG_WARN("No MAG agent for Binding Update List");
      
      return;
    }
    
  SetLastBindingUpdateTime (MicroSeconds (Simulator::Now ().GetMicroSeconds ()));
  SetLastBindingUpdateSequence (mag->GetSequence ());
 
  Ptr<Packet> p = mag->BuildPbu(this);
  
  SetPbuPacket(p);
  
  ResetRetryCount();
  
  mag->SendMessage(p->Copy(), GetLmaAddress(), 64);
  
  MarkRefreshing();
  
  StartRetransTimer();
}

void BindingUpdateList::Entry::FunctionReachableTimeout()
{
  NS_LOG_FUNCTION_NOARGS();
  
  Ptr<Pmipv6Mag> mag = m_buList->GetNode()->GetObject<Pmipv6Mag>();
  
  NS_LOG_LOGIC ("Reachable Timeout");
   
  if( mag == 0)
    {
      NS_LOG_WARN("No MAG agent for Binding Update List");
      
      return;
    }
    
  if(IsReachable())
    {
      MarkUnreachable();
    }
  else if(IsRefreshing())
    {
      MarkUpdating();
    }

  if( m_radvdIfIndex >= 0 )
    {
      mag->ClearRadvdInterface (this);
    }
      
  //delete routing && tunnel
  if (m_tunnelIfIndex >= 0)
    {
      mag->ClearTunnelAndRouting (this);
    }
}

void BindingUpdateList::Entry::FunctionRetransTimeout()
{
  NS_LOG_FUNCTION_NOARGS();
  Ptr<Pmipv6Mag> mag = m_buList->GetNode()->GetObject<Pmipv6Mag>();
  
  if( mag == 0)
    {
      NS_LOG_WARN("No MAG agent for Binding Update List");
      
      return;
    }
  
  IncreaseRetryCount();
  
  if ( GetRetryCount() > Ipv6MobilityL4Protocol::MAX_BINDING_UPDATE_RETRY_COUNT )
    {
      NS_LOG_LOGIC ("Maximum retry count reached. Giving up..");
      
      return;
    }
  
  mag->SendMessage(GetPbuPacket ()->Copy (), GetLmaAddress (), 64);
  
  StartRetransTimer();  
}

bool BindingUpdateList::Entry::IsUnreachable() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_state == UNREACHABLE;
}

bool BindingUpdateList::Entry::IsUpdating() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_state == UPDATING;
}

bool BindingUpdateList::Entry::IsRefreshing() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_state == REFRESHING;
}

bool BindingUpdateList::Entry::IsReachable() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_state == REACHABLE;
}

void BindingUpdateList::Entry::MarkUnreachable()
{
  NS_LOG_FUNCTION_NOARGS();
  
  m_state = UNREACHABLE;
}

void BindingUpdateList::Entry::MarkUpdating()
{
  NS_LOG_FUNCTION_NOARGS();
  
  m_state = UPDATING;
}

void BindingUpdateList::Entry::MarkRefreshing()
{
  NS_LOG_FUNCTION_NOARGS();
  
  m_state = REFRESHING;
}

void BindingUpdateList::Entry::MarkReachable()
{
  NS_LOG_FUNCTION_NOARGS();
  
  m_state = REACHABLE;
}
    
void BindingUpdateList::Entry::StartReachableTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT ( !m_reachableTime.IsZero() );
  
  m_reachableTimer.SetFunction (&BindingUpdateList::Entry::FunctionReachableTimeout, this);
  m_reachableTimer.SetDelay ( Seconds (m_reachableTime.GetSeconds ()));
  m_reachableTimer.Schedule ();
}

void BindingUpdateList::Entry::StopReachableTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_reachableTimer.Cancel ();
}

void BindingUpdateList::Entry::StartRetransTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_retransTimer.SetFunction (&BindingUpdateList::Entry::FunctionRetransTimeout, this);
  
  if (GetRetryCount () == 0)
    {
      m_retransTimer.SetDelay (Seconds (Ipv6MobilityL4Protocol::INITIAL_BINDING_ACK_TIMEOUT_FIRSTREG));
    }
  else
    {
      m_retransTimer.SetDelay (Seconds (Ipv6MobilityL4Protocol::INITIAL_BINDING_ACK_TIMEOUT_REREG));
    }
    
  m_retransTimer.Schedule ();
}

void BindingUpdateList::Entry::StopRetransTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_retransTimer.Cancel ();
}

void BindingUpdateList::Entry::StartRefreshTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  NS_ASSERT ( !m_reachableTime.IsZero() );
  
  m_refreshTimer.SetFunction (&BindingUpdateList::Entry::FunctionRefreshTimeout, this);
  m_refreshTimer.SetDelay ( Seconds ( m_reachableTime.GetSeconds() * 0.9 ) );
  m_refreshTimer.Schedule ();
}

void BindingUpdateList::Entry::StopRefreshTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_refreshTimer.Cancel ();
}

bool BindingUpdateList::Entry::Match(Identifier mnId) const
{
  NS_LOG_FUNCTION ( this << mnId );
  NS_ASSERT ( mnId == GetMnIdentifier() );

  return true;
}

Identifier BindingUpdateList::Entry::GetMnIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_mnIdentifier;
}

void BindingUpdateList::Entry::SetMnIdentifier(Identifier mnId)
{
  NS_LOG_FUNCTION ( this << mnId );
  
  m_mnIdentifier = mnId;
}

Identifier BindingUpdateList::Entry::GetMnLinkIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_mnLinkIdentifier;
}

void BindingUpdateList::Entry::SetMnLinkIdentifier(Identifier mnLinkId)
{
  NS_LOG_FUNCTION ( this << mnLinkId );
  
  m_mnLinkIdentifier = mnLinkId;
}

std::list<Ipv6Address> BindingUpdateList::Entry::GetHomeNetworkPrefixes() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_homeNetworkPrefixes;
}

void BindingUpdateList::Entry::SetHomeNetworkPrefixes(std::list<Ipv6Address> hnpList)
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_homeNetworkPrefixes = hnpList;
}

Ipv6Address BindingUpdateList::Entry::GetMagLinkAddress() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_magLinkAddress;
}

void BindingUpdateList::Entry::SetMagLinkAddress(Ipv6Address lla)
{
  NS_LOG_FUNCTION ( this << lla );
  
  m_magLinkAddress = lla;
}

Ipv6Address BindingUpdateList::Entry::GetLmaAddress() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_lmaAddress;
}

void BindingUpdateList::Entry::SetLmaAddress(Ipv6Address lmaa)
{
  NS_LOG_FUNCTION ( this << lmaa );
  
  m_lmaAddress = lmaa;
}

int16_t BindingUpdateList::Entry::GetIfIndex() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_ifIndex;
}

void BindingUpdateList::Entry::SetIfIndex(int16_t ifi)
{
  NS_LOG_FUNCTION ( this << ifi );
  
  m_ifIndex = ifi;
}

int16_t BindingUpdateList::Entry::GetTunnelIfIndex() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_tunnelIfIndex;
}

void BindingUpdateList::Entry::SetTunnelIfIndex(int16_t tunnelif)
{
  NS_LOG_FUNCTION ( this << tunnelif );
  
  m_tunnelIfIndex = tunnelif;
}

uint8_t BindingUpdateList::Entry::GetAccessTechnologyType() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_accessTechnologyType;
}

void BindingUpdateList::Entry::SetAccessTechnologyType(uint8_t att)
{
  NS_LOG_FUNCTION ( this << (uint32_t) att);
  
  m_accessTechnologyType = att;
}

uint8_t BindingUpdateList::Entry::GetHandoffIndicator() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_handoffIndicator;
}

void BindingUpdateList::Entry::SetHandoffIndicator(uint8_t hi)
{
  NS_LOG_FUNCTION ( this << (uint32_t) hi);
  
  m_handoffIndicator = hi;
}

Time BindingUpdateList::Entry::GetLastBindingUpdateTime() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_lastBindingUpdateTime;
}

void BindingUpdateList::Entry::SetLastBindingUpdateTime(Time tm)
{
  NS_LOG_FUNCTION ( this << tm );
  
  m_lastBindingUpdateTime = tm;
}

Time BindingUpdateList::Entry::GetReachableTime() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_reachableTime;
}

void BindingUpdateList::Entry::SetReachableTime(Time tm)
{
  NS_LOG_FUNCTION (this << tm );
  
  m_reachableTime = tm;
}

uint16_t BindingUpdateList::Entry::GetLastBindingUpdateSequence() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_lastBindingUpdateSequence;
}

void BindingUpdateList::Entry::SetLastBindingUpdateSequence(uint16_t seq)
{
  NS_LOG_FUNCTION( this << seq);
  
  m_lastBindingUpdateSequence = seq;
}

Ptr<Packet> BindingUpdateList::Entry::GetPbuPacket() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_pktPbu;
}

void BindingUpdateList::Entry::SetPbuPacket(Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION( this << pkt );
  
  m_pktPbu = pkt;
}

uint8_t BindingUpdateList::Entry::GetRetryCount() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_retryCount;
}

void BindingUpdateList::Entry::ResetRetryCount()
{
  NS_LOG_FUNCTION_NOARGS();
  
  m_retryCount = 0;
}

void BindingUpdateList::Entry::IncreaseRetryCount()
{
  NS_LOG_FUNCTION_NOARGS();
  
  m_retryCount++;
}

BindingUpdateList::Entry *BindingUpdateList::Entry::GetNext() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_next;
}

void BindingUpdateList::Entry::SetNext(BindingUpdateList::Entry *entry)
{
  NS_LOG_FUNCTION ( this << entry );
  
  m_next = entry;
}

int32_t BindingUpdateList::Entry::GetRadvdIfIndex() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_radvdIfIndex;
}

void BindingUpdateList::Entry::SetRadvdIfIndex(int32_t ifIndex)
{
  NS_LOG_FUNCTION ( this << ifIndex );
  
  m_radvdIfIndex = ifIndex;
}

int64_t BindingUpdateList::Entry::GetImsi () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_imsi;
}

void BindingUpdateList::Entry::SetImsi (int64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  m_imsi = imsi;
}

int32_t BindingUpdateList::Entry::GetTunnelId () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_teid;
}

void BindingUpdateList::Entry::SetTunnelId (int32_t teid)
{
  NS_LOG_FUNCTION (this << teid);
  m_teid = teid;
}

} /* namespace ns3 */
