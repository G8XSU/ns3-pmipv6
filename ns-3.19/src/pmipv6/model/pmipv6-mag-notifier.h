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

#ifndef PMIPV6_MAG_NOTIFIER_H
#define PMIPV6_MAG_NOTIFIER_H

#include "ns3/ipv6-address.h"
#include "ns3/mac48-address.h"
#include "ns3/ip-l4-protocol.h"

namespace ns3
{

class Node;
class Packet;

class Pmipv6MagNotifyHeader : public Header
{
public:
  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  Pmipv6MagNotifyHeader();
 
  virtual ~Pmipv6MagNotifyHeader();

  Mac48Address GetMacAddress() const;
  void SetMacAddress(Mac48Address macaddr);
  
  uint8_t GetAccessTechnologyType() const;
  void SetAccessTechnologyType(uint8_t att);
  
  virtual void Print (std::ostream& os) const;
  virtual uint32_t GetSerializedSize () const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
 
protected:

private:
  uint8_t m_nextHeader;
  uint8_t m_length;
  Mac48Address m_macAddress;
  uint8_t m_accessTechnologyType;
  uint8_t m_reserved[7];
};


class Pmipv6MagNotifier : public IpL4Protocol {
public:
  static TypeId GetTypeId ();
  static const uint8_t PROT_NUMBER;
  static uint16_t GetStaticProtocolNumber ();

  Pmipv6MagNotifier();
  
  virtual ~Pmipv6MagNotifier();
  
  void SetNode (Ptr<Node> node);

  Ptr<Node> GetNode (void);

  virtual int GetProtocolNumber () const;

  virtual void NotifyNewAggregate();

  void SendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl);
  
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p, Ipv4Header const &header, Ptr<Ipv4Interface> incomingInterface);
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p, Ipv6Header const &header, Ptr<Ipv6Interface> incomingInterface);

  virtual void SetDownTarget (IpL4Protocol::DownTargetCallback cb);
  virtual void SetDownTarget6 (IpL4Protocol::DownTargetCallback6 cb);
  virtual IpL4Protocol::DownTargetCallback GetDownTarget (void) const;
  virtual IpL4Protocol::DownTargetCallback6 GetDownTarget6 (void) const;
  
  void SetTargetAddress(Ipv6Address target);
  Ipv6Address GetTargetAddress();  
  
  void SetNewNodeCallback (Callback<void, Mac48Address, Mac48Address, uint8_t> cb);
  
protected:
  virtual void DoDispose ();

private:
  void HandleNewNode(Mac48Address from, Mac48Address to, uint8_t att);
  
  Ptr<Node> m_node;
  
  Ipv6Address m_targetAddress;
  
  Callback<void, Mac48Address, Mac48Address, uint8_t> m_newNodeCallback;
  
};

} /* namespace ns3 */

#endif /* PMIPV6_MAG_NOTIFIER_H */

