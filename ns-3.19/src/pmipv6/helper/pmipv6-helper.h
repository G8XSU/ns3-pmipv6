/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef PMIPV6_HELPER_H
#define PMIPV6_HELPER_H

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/trace-helper.h"

#include "ns3/identifier.h"

namespace ns3 {

class Node;
class Pmipv6ProfileHelper;
class Pmipv6Profile;

class Pmipv6LmaHelper {
public:
  Pmipv6LmaHelper();
  ~Pmipv6LmaHelper();
  /**
   * \param node The node on which to install the stack.
   */
  void Install (Ptr<Node> node) const;
  
  void SetProfileHelper(Ptr<Pmipv6ProfileHelper> pf);
  Ptr<Pmipv6ProfileHelper> GetProfileHelper ();
  
  void SetPrefixPoolBase(Ipv6Address prefixBegin, uint8_t prefixLen);

protected:

private:
  Ptr<Pmipv6ProfileHelper> m_profile;
  
  Ipv6Address m_prefixBegin;
  uint8_t m_prefixBeginLen;
};

class Pmipv6MagHelper {
public:
  Pmipv6MagHelper();
  ~Pmipv6MagHelper();
  
  /**
   * \param node The node on which to install the stack.
   */
  void Install (Ptr<Node> node, bool isLteMag = false) const;
  void Install (Ptr<Node> node, Ipv6Address target, NodeContainer aps) const;
  
  void SetProfileHelper (Ptr<Pmipv6ProfileHelper> pf);
  Ptr<Pmipv6ProfileHelper> GetProfileHelper ();
  
protected:

private:
  Ptr<Pmipv6ProfileHelper> m_profile;
};

class Pmipv6ProfileHelper : public SimpleRefCount<Pmipv6ProfileHelper>
{
public:
  Pmipv6ProfileHelper();
  ~Pmipv6ProfileHelper();
  
  Ptr<Pmipv6Profile> GetProfile();
  
  void AddProfile (Identifier mnId, Identifier mnLinkId, Ipv6Address lmaa, std::list<Ipv6Address> hnps, uint64_t imsi = 0);
protected:

private:
  Ptr<Pmipv6Profile> m_profile;
};

} // namespace ns3

#endif /* PMIPV6_HELPER_H */
