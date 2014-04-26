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

#include "ipv6-mobility-option-demux.h"
#include "ipv6-mobility-option.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityOptionDemux);

TypeId Ipv6MobilityOptionDemux::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityOptionDemux")
    .SetParent<Object> ()
    .AddAttribute ("MobilityOptions", "The set of IPv6 Mobility options registered with this demux.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&Ipv6MobilityOptionDemux::m_options),
                   MakeObjectVectorChecker<Ipv6MobilityOption> ())
    ;
  return tid;
}

Ipv6MobilityOptionDemux::Ipv6MobilityOptionDemux ()
{
}

Ipv6MobilityOptionDemux::~Ipv6MobilityOptionDemux ()
{
}

void Ipv6MobilityOptionDemux::DoDispose ()
{
  for (Ipv6MobilityOptionList_t::iterator it = m_options.begin (); it != m_options.end (); it++)
    {
      (*it)->Dispose ();
      *it = 0;
    }
  m_options.clear ();
  m_node = 0;
  Object::DoDispose ();
}

void Ipv6MobilityOptionDemux::SetNode (Ptr<Node> node)
{
  m_node = node;
}

void Ipv6MobilityOptionDemux::Insert (Ptr<Ipv6MobilityOption> option)
{
  m_options.push_back (option);
}

Ptr<Ipv6MobilityOption> Ipv6MobilityOptionDemux::GetOption (int optionNumber)
{
  for (Ipv6MobilityOptionList_t::iterator i = m_options.begin (); i != m_options.end (); ++i)
    {
      if ((*i)->GetMobilityOptionNumber () == optionNumber)
        {
          return *i;
        }
    }
  return 0;
}

void Ipv6MobilityOptionDemux::Remove (Ptr<Ipv6MobilityOption> option)
{
  m_options.remove (option);
}

} /* namespace ns3 */
