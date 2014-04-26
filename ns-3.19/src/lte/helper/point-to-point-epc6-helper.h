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

#ifndef POINT_TO_POINT_EPC6_HELPER_H
#define POINT_TO_POINT_EPC6_HELPER_H

#include <ns3/object.h>
#include <ns3/ipv6-address-helper.h>
#include <ns3/data-rate.h>
#include <ns3/epc-tft.h>
#include <ns3/eps-bearer.h>
#include <ns3/epc-helper.h>

namespace ns3 {

class Node;
class NetDevice;
class VirtualNetDevice;
class Epc6SgwPgwApplication;
class EpcX2;
class EpcMme;

/**
 * \brief Create an EPC network with PointToPoint links
 *
 * This Helper will create an EPC network topology comprising of a
 * single node that implements both the SGW and PGW functionality, and
 * an MME node. The S1-U, X2-U and X2-C interfaces are realized over
 * PointToPoint links. 
 */
class PointToPointEpc6Helper : public EpcHelper
{
public:
  
  /** 
   * Constructor
   */
  PointToPointEpc6Helper ();

  /** 
   * Destructor
   */  
  virtual ~PointToPointEpc6Helper ();
  
  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  // inherited from Epc6Helper
  virtual void AddEnb (Ptr<Node> enbNode, Ptr<NetDevice> lteEnbNetDevice, uint16_t cellId);
  virtual void AddUe (Ptr<NetDevice> ueLteDevice, uint64_t imsi);
  virtual void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2);
  virtual void ActivateEpsBearer (Ptr<NetDevice> ueLteDevice, uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer);
  virtual Ptr<Node> GetPgwNode ();
  virtual Ipv6InterfaceContainer AssignUeIpv6Address (NetDeviceContainer ueDevices);
  virtual Ipv6Address GetUeDefaultGatewayAddress ();

private:

  /**
   * SGW-PGW network element
   */

  /** 
   * helper to assign addresses to UE devices as well as to the TUN device of the SGW/PGW
   */
  Ipv6AddressHelper m_ueAddressHelper;
  
  Ptr<Node> m_sgwPgw; 
  Ptr<Epc6SgwPgwApplication> m_sgwPgwApp;
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

#endif // POINT_TO_POINT_EPC_HELPER_H
