/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008-2009 Strasbourg University
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
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/mac16-address.h"
#include "ns3/mac48-address.h"
#include "ns3/mac64-address.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-address-generator.h"

#include "ipv6-address-helper.h"

namespace ns3 
{

NS_LOG_COMPONENT_DEFINE ("Ipv6AddressHelper")
  ;

Ipv6AddressHelper::Ipv6AddressHelper ()
{
  NS_LOG_FUNCTION (this);
  SetBase (Ipv6Address ("2001:db8::"), Ipv6Prefix (32));
}

Ipv6AddressHelper::Ipv6AddressHelper (Ipv6Address network, Ipv6Prefix prefix,
                                      Ipv6Address base)
{
  NS_LOG_FUNCTION (this << network << prefix << base);
  SetBase (network, prefix, base);
}

void Ipv6AddressHelper::SetBase (Ipv6Address network, Ipv6Prefix prefix,
                            Ipv6Address base)
{
  NS_LOG_FUNCTION (this << network << prefix << base);
  m_network = network;
  /// \todo for now we do not enforce the prefix because the underlying
  /// Ipv6AddressGenerator does not handle prefixes well that are not 64 bits
  m_prefix = Ipv6Prefix (64);
  m_base = m_address = base;
  NS_LOG_DEBUG (this << "Network: " << m_network << ", Prefix: " << m_prefix << ", Base: " << m_base);
}


Ipv6Address Ipv6AddressHelper::NewAddress (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  if (Mac64Address::IsMatchingType (addr))
    {
      Ipv6Address address = Ipv6Address::MakeAutoconfiguredAddress (Mac64Address::ConvertFrom (addr), m_network);
      Ipv6AddressGenerator::AddAllocated (address);
      return address;
    }
  else if (Mac48Address::IsMatchingType (addr))
    {
      Ipv6Address address = Ipv6Address::MakeAutoconfiguredAddress (Mac48Address::ConvertFrom (addr), m_network);
      Ipv6AddressGenerator::AddAllocated (address);
      return address;
    }
  else if (Mac16Address::IsMatchingType (addr))
    {
      Ipv6Address address = Ipv6Address::MakeAutoconfiguredAddress (Mac16Address::ConvertFrom (addr), m_network);
      Ipv6AddressGenerator::AddAllocated (address);
      return address;
    }
  else
    {
      NS_FATAL_ERROR ("Did not pass in a valid Mac Address (16, 48 or 64 bits)");
    }
  /* never reached */
  return Ipv6Address ("::");
}

Ipv6Address Ipv6AddressHelper::NewAddress (void)
{
  NS_LOG_FUNCTION (this);

// The way this is expected to be used is that an address and network number
// are initialized, and then NewAddress() is called repeatedly to allocate and
// get new addresses on a given subnet.  The client will expect that the first
// address she gets back is the one she used to initialize the generator with.
// This implies that this operation is a post-increment.

  // Get the address to be assigned.
  Ipv6AddressGenerator::Init (m_network, m_prefix, m_address);
  Ipv6Address address = Ipv6AddressGenerator::NextAddress (m_prefix);

  // Update m_address with the next host address.
  Ipv6Address nextAddress = Ipv6AddressGenerator::GetAddress (m_prefix);
  uint8_t networkBuffer[16], addressBuffer[16], baseBuffer[16];
  m_network.GetBytes (networkBuffer);
  nextAddress.GetBytes (addressBuffer);
  // Extract the host part from the ipv6 address.
  for (uint8_t i = 0; i < 16; i++)
    {
      baseBuffer[i] = networkBuffer[i] ^ addressBuffer[i];
    }
  m_address.Set (baseBuffer);

  return address;
}

void Ipv6AddressHelper::NewNetwork (Ipv6Address network, Ipv6Prefix prefix)
{
  NS_LOG_FUNCTION (this << network << prefix);
  SetBase (network, prefix);
}

void Ipv6AddressHelper::NewNetwork (void)
{
  NS_LOG_FUNCTION (this);
  Ipv6AddressGenerator::Init (m_network, m_prefix, m_address);
  Ipv6AddressGenerator::NextNetwork (m_prefix);
  SetBase (Ipv6AddressGenerator::GetNetwork (m_prefix), m_prefix, m_base);
}

Ipv6InterfaceContainer Ipv6AddressHelper::Assign (const NetDeviceContainer &c)
{
  NS_LOG_FUNCTION (this);
  Ipv6InterfaceContainer retval;

  for (uint32_t i = 0; i < c.GetN (); ++i) 
    {
      Ptr<NetDevice> device = c.Get (i);

      Ptr<Node> node = device->GetNode ();
      NS_ASSERT_MSG (node, "Ipv6AddressHelper::Allocate (): Bad node");

      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
      NS_ASSERT_MSG (ipv6, "Ipv6AddressHelper::Allocate (): Bad ipv6");
      int32_t ifIndex = 0;

      ifIndex = ipv6->GetInterfaceForDevice (device);
      if (ifIndex == -1)
        {
          ifIndex = ipv6->AddInterface (device);
        }
      NS_ASSERT_MSG (ifIndex >= 0, "Ipv6AddressHelper::Allocate (): "
                     "Interface index not found");

      Ipv6InterfaceAddress ipv6Addr = Ipv6InterfaceAddress (NewAddress (device->GetAddress ()), m_prefix);
      ipv6->SetMetric (ifIndex, 1);
      ipv6->AddAddress (ifIndex, ipv6Addr);
      ipv6->SetUp (ifIndex);

      retval.Add (ipv6, ifIndex);
    }
  return retval;
}

Ipv6InterfaceContainer Ipv6AddressHelper::Assign (const NetDeviceContainer &c, std::vector<bool> withConfiguration)
{
  NS_LOG_FUNCTION (this);
  Ipv6InterfaceContainer retval;
  for (uint32_t i = 0; i < c.GetN (); ++i) 
    {
      Ptr<NetDevice> device = c.Get (i);

      Ptr<Node> node = device->GetNode ();
      NS_ASSERT_MSG (node, "Ipv6AddressHelper::Allocate (): Bad node");

      Ptr<Ipv6> ipv6 = node->GetObject<Ipv6> ();
      NS_ASSERT_MSG (ipv6, "Ipv6AddressHelper::Allocate (): Bad ipv6");

      int32_t ifIndex = ipv6->GetInterfaceForDevice (device);
      if (ifIndex == -1)
        {
          ifIndex = ipv6->AddInterface (device);
        }
      NS_ASSERT_MSG (ifIndex >= 0, "Ipv6AddressHelper::Allocate (): "
                     "Interface index not found");

      ipv6->SetMetric (ifIndex, 1);

      if (withConfiguration.at (i))
        {
          Ipv6InterfaceAddress ipv6Addr = Ipv6InterfaceAddress (NewAddress (device->GetAddress ()), m_prefix);
          ipv6->AddAddress (ifIndex, ipv6Addr);
        }

      ipv6->SetUp (ifIndex);
      retval.Add (ipv6, ifIndex);
    }
  return retval;
}

// Helper API that is redundant with Assign (c, false);
Ipv6InterfaceContainer Ipv6AddressHelper::AssignWithoutAddress (const NetDeviceContainer &c)
{
  NS_LOG_FUNCTION (this);
  std::vector<bool> withConfiguration;
  for (uint32_t i = 0; i < c.GetN (); ++i) 
    {
      withConfiguration.push_back (false);
    }
  return Assign (c, withConfiguration);
}

} /* namespace ns3 */

