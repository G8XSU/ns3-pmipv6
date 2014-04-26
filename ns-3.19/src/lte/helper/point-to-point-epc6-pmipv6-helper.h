/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Manuel Requena <manuel.requena@cttc.es>
 */

#ifndef POINT_TO_POINT_EPC6_PMIPV6_HELPER_H
#define POINT_TO_POINT_EPC6_PMIPV6_HELPER_H

#include <ns3/object.h>
#include <ns3/ipv6-address-helper.h>
#include <ns3/data-rate.h>
#include <ns3/epc-tft.h>
#include <ns3/eps-bearer.h>
#include <ns3/epc-helper.h>
#include "ns3/pmipv6-helper.h"

namespace ns3 {

class Node;
class NetDevice;
class VirtualNetDevice;
class Epc6SgwApplication;
class EpcX2;
class EpcMme;

/**
 * \brief Create an EPC network with PointToPoint links
 *
 * This Helper will create an EPC network topology comprising of a
 * node that implements the SGW and another node with PGW functionality,
 * and an MME node. The S1-U, S5, X2-U and X2-C interfaces are realized
 * over PointToPoint links. The S5 link uses PMIPv6 as the mobility protocol.
 */
class PointToPointEpc6Pmipv6Helper : public EpcHelper
{
public:
  
  /** 
   * Constructors
   */
  PointToPointEpc6Pmipv6Helper ();
  PointToPointEpc6Pmipv6Helper (Mac48Address tunDevMacAddress);

  /** 
   * Destructor
   */  
  virtual ~PointToPointEpc6Pmipv6Helper ();
  
  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  // inherited from EpcHelper
  virtual void AddEnb (Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice, uint16_t cellId);
  virtual void AddUe (Ptr<NetDevice> ueLteDevice, uint64_t imsi);
  virtual void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2);
  virtual void ActivateEpsBearer (Ptr<NetDevice> ueLteDevice, uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer);
  virtual Ptr<Node> GetPgwNode ();

  Ipv6InterfaceContainer AssignWithoutAddress (NetDeviceContainer ueDevices);
  virtual Ptr<Node> GetSgwNode ();
  void SetupS5Interface ();
  Ptr<Pmipv6ProfileHelper> GetPmipv6ProfileHelper ();

private:

  void Initialize (Mac48Address tunDevMacAddress);
  /**
   * SGW network element
   */
  
  Ptr<Node> m_sgw;
  Ptr<Node> m_pgw;
  Ptr<Epc6SgwApplication> m_sgwApp;
  Ptr<VirtualNetDevice> m_tunDevice;
  Ptr<EpcMme> m_mme;

  /**
   * S1-U interfaces
   */

  /** 
   * helper to assign addresses to S1-U NetDevices 
   */
  Ipv6AddressHelper m_s1uIpv6AddressHelper;

  DataRate m_s1uLinkDataRate;
  Time     m_s1uLinkDelay;
  uint16_t m_s1uLinkMtu;

  Ipv6AddressHelper m_s5Ipv6AddressHelper;

  DataRate m_s5LinkDataRate;
  Time     m_s5LinkDelay;
  uint16_t m_s5LinkMtu;

  Ptr<Pmipv6ProfileHelper> m_pmipv6ProfileHelper;
  Pmipv6LmaHelper m_pmipv6LmaHelper;
  Pmipv6MagHelper m_pmipv6MagHelper;

  /**
   * UDP port where the GTP-U Socket is bound, fixed by the standard as 2152
   */
  uint16_t m_gtpuUdpPort;

  /**
   * Map storing for each IMSI the corresponding eNB NetDevice
   * 
   */
  std::map<uint64_t, Ptr<NetDevice> > m_imsiEnbDeviceMap;
  
  /** 
   * helper to assign addresses to X2 NetDevices 
   */
  Ipv6AddressHelper m_x2Ipv6AddressHelper;

  DataRate m_x2LinkDataRate;
  Time     m_x2LinkDelay;
  uint16_t m_x2LinkMtu;

};

} // namespace ns3

#endif // POINT_TO_POINT_EPC6_PMIPV6_HELPER_H
