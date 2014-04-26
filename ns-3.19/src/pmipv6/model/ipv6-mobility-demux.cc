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

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/object-vector.h"

#include "ipv6-mobility-demux.h"
#include "ipv6-mobility.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityDemux);

TypeId Ipv6MobilityDemux::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityDemux")
    .SetParent<Object> ()
    .AddAttribute ("Mobilities", "The set of IPv6 Mobilities registered with this demux.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&Ipv6MobilityDemux::m_mobilities),
                   MakeObjectVectorChecker<Ipv6Mobility> ())
    ;
  return tid;
}

Ipv6MobilityDemux::Ipv6MobilityDemux ()
{
}

Ipv6MobilityDemux::~Ipv6MobilityDemux ()
{
}

void Ipv6MobilityDemux::DoDispose ()
{
  for (Ipv6MobilityList_t::iterator it = m_mobilities.begin (); it != m_mobilities.end (); it++)
    {
      (*it)->Dispose ();
      *it = 0;
    }
  m_mobilities.clear ();
  m_node = 0;
  Object::DoDispose ();
}

void Ipv6MobilityDemux::SetNode (Ptr<Node> node)
{
  m_node = node;
}

void Ipv6MobilityDemux::Insert (Ptr<Ipv6Mobility> mobility)
{
  m_mobilities.push_back (mobility);
}

Ptr<Ipv6Mobility> Ipv6MobilityDemux::GetMobility (int mobilityNumber)
{
  for (Ipv6MobilityList_t::iterator i = m_mobilities.begin (); i != m_mobilities.end (); ++i)
    {
      if ((*i)->GetMobilityNumber () == mobilityNumber)
        {
          return *i;
        }
    }
  return 0;
}

void Ipv6MobilityDemux::Remove (Ptr<Ipv6Mobility> mobility)
{
  m_mobilities.remove (mobility);
}

} /* namespace ns3 */
