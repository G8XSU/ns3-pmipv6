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

#include "ipv6-mobility-header.h"
#include "ipv6-mobility.h"

#include "ipv6-mobility-option-header.h"
#include "ipv6-mobility-option.h"

#include "ipv6-mobility-l4-protocol.h"

#include "pmipv6-lma.h"

#include "binding-cache.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("BindingCache");

TypeId BindingCache::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BindingCache")
    .SetParent<Object> ()
    ;
  return tid;
} 

BindingCache::BindingCache ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

BindingCache::~BindingCache ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Flush ();
}

void BindingCache::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Flush ();
  Object::DoDispose ();
}

BindingCache::Entry *BindingCache::Lookup (Identifier mnId, std::list<Ipv6Address> hnpList, bool &allMatched)
{
  NS_LOG_FUNCTION (this << mnId);
  if ( m_bCache.find (mnId) != m_bCache.end ())
    {
      BindingCache::Entry* entry = m_bCache[mnId];
      BindingCache::Entry* partial = 0;
      bool matchtemp;

      while (entry)
        {
          matchtemp = false;
          if (entry->Match (mnId, hnpList, matchtemp))
            {
              if (matchtemp)
                {
                  allMatched = true;
                  return entry;
                }
              if (partial == 0)
                {
                  partial = entry;
                }
            }
          entry = entry->GetNext();
        }
      allMatched = false;
      return partial;
    }
  return 0;
}

BindingCache::Entry *BindingCache::Lookup(Identifier mnId, uint8_t att, Identifier mnLinkId)
{
  NS_LOG_FUNCTION (this << mnId << mnLinkId);
  
  if (m_bCache.find (mnId) != m_bCache.end ())
    {
      BindingCache::Entry* entry = m_bCache[mnId];
      while (entry)
        {
          if (entry->Match (mnId, att, mnLinkId))
            {
                  return entry;
            }
          entry = entry->GetNext ();
        }
    }
  return 0;
}

BindingCache::Entry *BindingCache::Lookup (Identifier mnId)
{
  NS_LOG_FUNCTION (this << mnId );
  
  if (m_bCache.find (mnId) != m_bCache.end ())
    {
      BindingCache::Entry* entry = m_bCache[mnId];
      return entry;
    }
  return 0;
}

BindingCache::Entry* BindingCache::Add (Identifier mnId)
{
  NS_LOG_FUNCTION (this << mnId );
  
  BindingCache::Entry* entry = new BindingCache::Entry (this);
  entry->SetMnIdentifier(mnId);
  // Add to beginning of list if entry with same MN Id already exists.
  if (m_bCache.find (mnId) != m_bCache.end ())
    {
      BindingCache::Entry* entry2 = m_bCache[mnId];
      entry->SetNext(entry2);
    }
  m_bCache[mnId] = entry;
  return entry;
}

void BindingCache::Remove (BindingCache::Entry* entry)
{
  NS_LOG_FUNCTION_NOARGS ();

  for (BCacheI i = m_bCache.begin () ; i != m_bCache.end () ; i++)
    {
      if ((*i).second == entry)
        {
          m_bCache.erase (i);
          delete entry;
          return;
        }
    }
}

void BindingCache::Flush ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (BCacheI i = m_bCache.begin () ; i != m_bCache.end () ; i++)
    {
      delete (*i).second; /* delete the pointer BindingCache::Entry */
    }
  m_bCache.erase (m_bCache.begin (), m_bCache.end ());
}

Ptr<Node> BindingCache::GetNode() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_node;
}

void BindingCache::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION ( this << node );
  
  m_node = node;
}

BindingCache::Entry::Entry (Ptr<BindingCache> bcache)
  : m_bCache (bcache),
    m_state (UNREACHABLE),
    m_tunnelIfIndex (-1),
    m_reachableTimer(Timer::CANCEL_ON_DESTROY),
    m_deregisterTimer(Timer::CANCEL_ON_DESTROY),
    m_registerTimer(Timer::CANCEL_ON_DESTROY),
    m_next (0),
    m_tentativeEntry (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

BindingCache::Entry::~Entry ()
{
  BindingCache::Entry* entry;
  entry = GetNext ();
  if (entry)
    delete entry;
  entry = GetTentativeEntry ();
  if (entry)
    delete entry;
}

BindingCache::Entry *BindingCache::Entry::Copy()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  Entry *bce = new Entry(this->m_bCache);

  bce->SetMnIdentifier(this->GetMnIdentifier());
  bce->SetMnLinkIdentifier(this->GetMnLinkIdentifier());
  bce->SetHomeNetworkPrefixes(this->GetHomeNetworkPrefixes());
  bce->SetMagLinkAddress(this->GetMagLinkAddress());
  bce->SetProxyCoa(this->GetProxyCoa());
  bce->SetTunnelIfIndex(this->GetTunnelIfIndex());
  bce->SetAccessTechnologyType(this->GetAccessTechnologyType());
  bce->SetHandoffIndicator(this->GetHandoffIndicator());
  bce->SetLastBindingUpdateTime(this->GetLastBindingUpdateTime());
  bce->SetReachableTime(this->GetReachableTime());
  bce->SetLastBindingUpdateSequence(this->GetLastBindingUpdateSequence());
  
  bce->SetNext(0);
  bce->SetTentativeEntry(0);

  return bce;
}

bool BindingCache::Entry::IsUnreachable() const
{
  NS_LOG_FUNCTION_NOARGS();

  return m_state == UNREACHABLE;  
}
bool BindingCache::Entry::IsEqual (BindingCache::Entry *bceToCheck){
  NS_LOG_FUNCTION_NOARGS();
  /*
   * checking
   * 1. att
   * 2. LastBindingUpdateSequence
   * 3.MagLinkAddress
   * 4.LastbindingUpdateTime
   * 5.HNPsList
   */
  if(bceToCheck->GetAccessTechnologyType()==this->GetAccessTechnologyType() && bceToCheck->GetLastBindingUpdateSequence()==this->GetLastBindingUpdateSequence() && bceToCheck->GetMagLinkAddress()==this->GetMagLinkAddress() && bceToCheck->GetLastBindingUpdateTime()==this->GetLastBindingUpdateTime() && bceToCheck->GetHomeNetworkPrefixes()==this->GetHomeNetworkPrefixes())
	  return true;
  else
	  return false;
}
bool BindingCache::Entry::IsDeregistering() const
{
  NS_LOG_FUNCTION_NOARGS();

  return m_state == DEREGISTERING;  
}

bool BindingCache::Entry::IsRegistering() const
{
  NS_LOG_FUNCTION_NOARGS();

  return m_state == REGISTERING;  
}

bool BindingCache::Entry::IsReachable() const
{
  NS_LOG_FUNCTION_NOARGS();

  return m_state == REACHABLE;  
}

void BindingCache::Entry::MarkUnreachable()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_state = UNREACHABLE;
}

void BindingCache::Entry::MarkDeregistering()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_state = DEREGISTERING;
}

void BindingCache::Entry::MarkRegistering ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_state = REGISTERING;
}

void BindingCache::Entry::MarkReachable()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_state = REACHABLE;
}
	
void BindingCache::Entry::FunctionReachableTimeout ()
{
  NS_LOG_FUNCTION_NOARGS();

//  Ptr<Pmipv6Lma> lma = m_bCache->GetNode()->GetObject<Pmipv6Lma>();
}

void BindingCache::Entry::FunctionDeregisterTimeout ()
{
  NS_LOG_FUNCTION_NOARGS();

//  Ptr<Pmipv6Lma> lma = m_bCache->GetNode()->GetObject<Pmipv6Lma>();
}

void BindingCache::Entry::FunctionRegisterTimeout ()
{
  NS_LOG_FUNCTION_NOARGS();

  Ptr<Pmipv6Lma> lma = m_bCache->GetNode ()->GetObject<Pmipv6Lma> ();
  NS_LOG_LOGIC ("Timeout waiting for previous MAG's de-registration.");
  lma->DoDelayedRegistration (this);
}

void BindingCache::Entry::StartReachableTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (!m_reachableTime.IsZero ());
  
  m_reachableTimer.SetFunction (&BindingCache::Entry::FunctionReachableTimeout, this);
  m_reachableTimer.SetDelay (m_reachableTime);
  m_reachableTimer.Schedule ();
}

void BindingCache::Entry::StopReachableTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_reachableTimer.Cancel ();
}

void BindingCache::Entry::StartDeregisterTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_deregisterTimer.SetFunction (&BindingCache::Entry::FunctionDeregisterTimeout, this);
  m_deregisterTimer.SetDelay ( MilliSeconds (Ipv6MobilityL4Protocol::MIN_DELAY_BEFORE_BCE_DELETE) );
  m_deregisterTimer.Schedule ();
}

void BindingCache::Entry::StopDeregisterTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_deregisterTimer.Cancel ();
}

void BindingCache::Entry::StartRegisterTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_registerTimer.SetFunction (&BindingCache::Entry::FunctionRegisterTimeout, this);
  m_registerTimer.SetDelay ( MilliSeconds (Ipv6MobilityL4Protocol::MIN_DELAY_BEFORE_NEW_BCE_ASSIGN) );
  m_registerTimer.Schedule ();
}

void BindingCache::Entry::StopRegisterTimer ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_registerTimer.Cancel ();
}

bool BindingCache::Entry::Match(Identifier mnId, std::list<Ipv6Address> hnpList, bool &allMatched) const
{
  NS_LOG_FUNCTION ( this << mnId );
  NS_ASSERT ( mnId == GetMnIdentifier() );
  
  if( hnpList.size() == 0 || m_homeNetworkPrefixes.size() == 0 )
    {
      allMatched = false;
      return false;
    }
  
  allMatched = true;
  
  bool found = false;
  bool eachfound;
  for (std::list<Ipv6Address>::const_iterator i = hnpList.begin(); i != hnpList.end(); i++)
    {
      eachfound = false;
      for (std::list<Ipv6Address>::const_iterator j = m_homeNetworkPrefixes.begin(); j != m_homeNetworkPrefixes.end(); j++)
        {
          if ((*i) == (*j))
            {
              found = true;
              eachfound = true;
            }
        }
      if (!eachfound)
        {
          allMatched = false;
        }
    }
  return found;
}

bool BindingCache::Entry::Match(Identifier mnId, uint8_t att, Identifier mnLinkId) const
{
  NS_LOG_FUNCTION (this << mnId << (uint32_t) att << mnLinkId);

  NS_ASSERT (mnId == GetMnIdentifier ());
  if (GetAccessTechnologyType() != att)
    {
      return false;
    }
  if (GetMnLinkIdentifier() != mnLinkId)
    {
      return false;
    }
  return true;
}

Identifier BindingCache::Entry::GetMnIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mnIdentifier;
}

void BindingCache::Entry::SetMnIdentifier(Identifier mnId)
{
  NS_LOG_FUNCTION (this << mnId);
  m_mnIdentifier = mnId;
}

Identifier BindingCache::Entry::GetMnLinkIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mnLinkIdentifier;
}

void BindingCache::Entry::SetMnLinkIdentifier(Identifier mnLinkId)
{
  NS_LOG_FUNCTION (this << mnLinkId);
  m_mnLinkIdentifier = mnLinkId;
}

std::list<Ipv6Address> BindingCache::Entry::GetHomeNetworkPrefixes() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_homeNetworkPrefixes;
}

void BindingCache::Entry::SetHomeNetworkPrefixes(std::list<Ipv6Address> hnpList)
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_homeNetworkPrefixes = hnpList;
}

Ipv6Address BindingCache::Entry::GetMagLinkAddress() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_magLinkAddress;
}

void BindingCache::Entry::SetMagLinkAddress(Ipv6Address lla)
{
  NS_LOG_FUNCTION ( this << lla );
  
  m_magLinkAddress = lla;
}

Ipv6Address BindingCache::Entry::GetProxyCoa() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_proxyCoa;
}

void BindingCache::Entry::SetProxyCoa(Ipv6Address pcoa)
{
  NS_LOG_FUNCTION ( this << pcoa );

  m_oldProxyCoa = m_proxyCoa;
  m_proxyCoa = pcoa;
  return;
}

int16_t BindingCache::Entry::GetTunnelIfIndex() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_tunnelIfIndex;
}

void BindingCache::Entry::SetTunnelIfIndex(int16_t tunnelif)
{
  NS_LOG_FUNCTION ( this << tunnelif );
  
  m_tunnelIfIndex = tunnelif;
}

uint8_t BindingCache::Entry::GetAccessTechnologyType() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_accessTechnologyType;
}

void BindingCache::Entry::SetAccessTechnologyType(uint8_t att)
{
  NS_LOG_FUNCTION ( this << (uint32_t) att);
  
  m_accessTechnologyType = att;
}

uint8_t BindingCache::Entry::GetHandoffIndicator() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_handoffIndicator;
}

void BindingCache::Entry::SetHandoffIndicator(uint8_t hi)
{
  NS_LOG_FUNCTION ( this << (uint32_t) hi);
  
  m_handoffIndicator = hi;
}

Time BindingCache::Entry::GetLastBindingUpdateTime() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_lastBindingUpdateTime;
}

void BindingCache::Entry::SetLastBindingUpdateTime(Time tm)
{
  NS_LOG_FUNCTION ( this << tm );
  
  m_lastBindingUpdateTime = tm;
}

Time BindingCache::Entry::GetReachableTime() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_reachableTime;
}

void BindingCache::Entry::SetReachableTime(Time tm)
{
  NS_LOG_FUNCTION (this << tm );
  
  m_reachableTime = tm;
}

uint16_t BindingCache::Entry::GetLastBindingUpdateSequence() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_lastBindingUpdateSequence;
}

void BindingCache::Entry::SetLastBindingUpdateSequence(uint16_t seq)
{
  NS_LOG_FUNCTION( this << seq);
  
  m_lastBindingUpdateSequence = seq;
}

BindingCache::Entry *BindingCache::Entry::GetNext() const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_next;
}

void BindingCache::Entry::SetNext (BindingCache::Entry *entry)
{
  NS_LOG_FUNCTION (this << entry);
  m_next = entry;
}
	
BindingCache::Entry *BindingCache::Entry::GetTentativeEntry () const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_tentativeEntry;
}

void BindingCache::Entry::SetTentativeEntry (BindingCache::Entry *entry)
{
  NS_LOG_FUNCTION ( this << entry );
  m_tentativeEntry = entry;
}

Ipv6Address BindingCache::Entry::GetOldProxyCoa () const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_oldProxyCoa;
}

} /* namespace ns3 */
