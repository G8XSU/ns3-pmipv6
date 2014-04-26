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

#include "pmipv6-profile.h"

NS_LOG_COMPONENT_DEFINE ("Pmipv6Profile");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Pmipv6Profile);

TypeId Pmipv6Profile::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Pmipv6Profile")
    .SetParent<Object> ()
	.AddConstructor<Pmipv6Profile>()
    ;
  return tid;
} 

Pmipv6Profile::Pmipv6Profile ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

Pmipv6Profile::~Pmipv6Profile ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Flush ();
}

void Pmipv6Profile::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Flush ();
  Object::DoDispose ();
}

Pmipv6Profile::Entry* Pmipv6Profile::LookupMnId (Identifier mnId)
{
  NS_LOG_FUNCTION (this << mnId);
  
  if (m_mnIdProfileList.find (mnId) != m_mnIdProfileList.end ())
    {
      Pmipv6Profile::Entry* entry = m_mnIdProfileList[mnId];
      return entry;
    }
  return 0;
}

Pmipv6Profile::Entry* Pmipv6Profile::LookupMnLinkId (Identifier mnLinkId)
{
  NS_LOG_FUNCTION (this << mnLinkId);

  if (m_mnLinkIdProfileList.find (mnLinkId) != m_mnLinkIdProfileList.end ())
    {
      Pmipv6Profile::Entry* entry = m_mnLinkIdProfileList[mnLinkId];
      return entry;
    }
  return 0;
}

Pmipv6Profile::Entry* Pmipv6Profile::LookupImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);

  if (m_imsiProfileList.find (imsi) != m_imsiProfileList.end ())
    {
      Pmipv6Profile::Entry* entry = m_imsiProfileList[imsi];
      return entry;
    }
  return 0;
}

Pmipv6Profile::Entry* Pmipv6Profile::AddMnId (Identifier mnId, Pmipv6Profile::Entry *entry)
{
  NS_LOG_FUNCTION (this << mnId);
  NS_ASSERT (m_mnIdProfileList.find (mnId) == m_mnIdProfileList.end());
  // Create entry if an existing entry isn't passed.
  if (entry == NULL)
    entry = new Pmipv6Profile::Entry (this);
  m_mnIdProfileList[mnId] = entry;
  return entry;
}

Pmipv6Profile::Entry* Pmipv6Profile::AddMnLinkId (Identifier mnLinkId, Pmipv6Profile::Entry *entry)
{
  NS_LOG_FUNCTION (this << mnLinkId);
  NS_ASSERT (m_mnLinkIdProfileList.find (mnLinkId) == m_mnLinkIdProfileList.end());
  // Create entry if an existing entry isn't passed.
  if (entry == NULL)
    entry = new Pmipv6Profile::Entry (this);
  m_mnLinkIdProfileList[mnLinkId] = entry;
  return entry;
}

Pmipv6Profile::Entry* Pmipv6Profile::AddImsi (uint64_t imsi, Pmipv6Profile::Entry *entry)
{
  NS_LOG_FUNCTION (this << imsi);
  NS_ASSERT (m_imsiProfileList.find (imsi) == m_imsiProfileList.end());
  // Create entry if an existing entry isn't passed.
  if (entry == NULL)
    entry = new Pmipv6Profile::Entry (this);
  m_imsiProfileList[imsi] = entry;
  return entry;
}

void Pmipv6Profile::Remove (Pmipv6Profile::Entry* entry)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (entry != NULL);
  Identifier mnId = entry->GetMnIdentifier ();
  MnIdProfileListI mnIt = m_mnIdProfileList.find (mnId);
  if (mnIt != m_mnIdProfileList.end ())
    m_mnIdProfileList.erase (mnIt);

  MnLinkIdProfileListI mnLinkIt = m_mnLinkIdProfileList.find (entry->GetMnLinkIdentifier ());
  if (mnLinkIt != m_mnLinkIdProfileList.end ())
    m_mnLinkIdProfileList.erase (mnLinkIt);

  ImsiProfileListI imsiIt = m_imsiProfileList.find (entry->GetImsi ());
  if (imsiIt != m_imsiProfileList.end ())
    m_imsiProfileList.erase (imsiIt);

  delete entry;
}

void Pmipv6Profile::Flush ()
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_INFO (m_mnIdProfileList.size ());
  for (MnIdProfileListI i = m_mnIdProfileList.begin () ; i != m_mnIdProfileList.end () ; i++)
    {
      delete (*i).second;
    }
  m_mnIdProfileList.erase (m_mnIdProfileList.begin (), m_mnIdProfileList.end ());
  m_mnLinkIdProfileList.erase (m_mnLinkIdProfileList.begin (), m_mnLinkIdProfileList.end ());
  m_imsiProfileList.erase (m_imsiProfileList.begin (), m_imsiProfileList.end ());
}

Pmipv6Profile::Entry::Entry (Pmipv6Profile* pf)
  : m_profile (pf)
{
  NS_LOG_FUNCTION_NOARGS ();
}

Identifier Pmipv6Profile::Entry::GetMnIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_mnIdentifier;
}

void Pmipv6Profile::Entry::SetMnIdentifier(Identifier mnId)
{
  NS_LOG_FUNCTION ( this << mnId );
  
  m_mnIdentifier = mnId;
}

Identifier Pmipv6Profile::Entry::GetMnLinkIdentifier() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_mnLinkIdentifier;
}

void Pmipv6Profile::Entry::SetMnLinkIdentifier(Identifier mnLinkId)
{
  NS_LOG_FUNCTION ( this << mnLinkId );
  
  m_mnLinkIdentifier = mnLinkId;
}

uint64_t Pmipv6Profile::Entry::GetImsi () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_imsi;
}

void Pmipv6Profile::Entry::SetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  m_imsi = imsi;
}

std::list<Ipv6Address> Pmipv6Profile::Entry::GetHomeNetworkPrefixes() const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_homeNetworkPrefixes;
}

void Pmipv6Profile::Entry::SetHomeNetworkPrefixes(std::list<Ipv6Address> hnps)
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_homeNetworkPrefixes = hnps;
}

void Pmipv6Profile::Entry::AddHomeNetworkPrefix(Ipv6Address hnp)
{
  NS_LOG_FUNCTION (this << hnp);
  
  m_homeNetworkPrefixes.push_back(hnp);
}

Ipv6Address Pmipv6Profile::Entry::GetLmaAddress() const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_lmaAddress;
}

void Pmipv6Profile::Entry::SetLmaAddress(Ipv6Address lmaa)
{
  NS_LOG_FUNCTION ( this << lmaa );
  
  m_lmaAddress = lmaa;
}

} /* namespace ns3 */
