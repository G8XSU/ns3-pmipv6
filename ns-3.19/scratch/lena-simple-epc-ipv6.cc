/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/flow-monitor-module.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
NS_LOG_COMPONENT_DEFINE ("EpcIpv6Example1");

void PrintCompleteNodeInfo(Ptr<Node> node)
{
  int n_interfaces, n_ipaddrs, i, j;
  Ptr<Ipv6> ipv6;
  Ipv6InterfaceAddress ipv6address;

  ipv6 = node->GetObject<Ipv6> ();
  n_interfaces = ipv6->GetNInterfaces();
  NS_LOG_INFO("No of interfaces: " << n_interfaces);
  for (i = 0; i < n_interfaces; i++)
  {
    n_ipaddrs = ipv6->GetNAddresses(i);
    NS_LOG_INFO("Interface " << i);
    for (j = 0; j < n_ipaddrs; j++)
    {
      ipv6address = ipv6->GetAddress(i, j);
      NS_LOG_INFO(ipv6address);
    }
  }
  OutputStreamWrapper osw = OutputStreamWrapper (&std::cout);
  Ptr<Ipv6RoutingProtocol> ipv6rp = ipv6->GetRoutingProtocol();
  ipv6rp->PrintRoutingTable(&osw);
}

void RxTrace (std::string context, Ptr<const Packet> packet, Ptr<Ipv6> ipv6, uint32_t interfaceId)
{
  NS_LOG_INFO (context << " " << interfaceId);
}

void TxTrace (std::string context, Ptr<const Packet> packet, Ptr<Ipv6> ipv6, uint32_t interfaceId)
{
  NS_LOG_INFO (context << " " << interfaceId);
}

void DropTrace (std::string context, const Ipv6Header & ipv6Header, Ptr<const Packet> packet, Ipv6L3Protocol::DropReason dropReason, Ptr<Ipv6> ipv6, uint32_t interfaceId)
{
  NS_LOG_INFO (context << " " << ipv6Header.GetSourceAddress () << " " << ipv6Header.GetDestinationAddress () << " " << dropReason << " " << interfaceId);
}

void PacketSinkRxTrace (std::string context, Ptr<const Packet> packet, const Address &address)
{
  SeqTsHeader seqTs;
  packet->Copy ()->RemoveHeader (seqTs);
  NS_LOG_UNCOND (context << " " << seqTs.GetTs () << "->" << Simulator::Now() << ": " << seqTs.GetSeq());
}

//void SendOutgoing (std::string context, const Ipv6Header &ipHeader, Ptr<const Packet> ipPayload, uint32_t interface)
//{
////  NS_LOG_UNCOND (context << " " << ipHeader << " " << interface);
////  std::cout << "Packet Tags: ";
////  ipPayload->PrintPacketTags (std::cout);
////  std::cout << std::endl;
//
//  Ipv6FlowProbeTag fTag;
//  bool found = ipPayload->PeekPacketTag (fTag);
//  if (found)
//    NS_LOG_UNCOND (context << " Tag found : " << fTag.GetFlowId () << " " << fTag.GetPacketId () << " " << fTag.GetPacketSize ());
//}
//
//void UnicastForward (std::string context, const Ipv6Header &ipHeader, Ptr<const Packet> ipPayload, uint32_t interface)
//{
//  NS_LOG_UNCOND (context << " " << ipHeader << " " << interface);
//  Ipv6FlowProbeTag fTag;
//  bool found = ipPayload->PeekPacketTag (fTag);
//  if (found)
//    NS_LOG_UNCOND (context << " Tag found : " << fTag.GetFlowId () << " " << fTag.GetPacketId () << " " << fTag.GetPacketSize ());
//}
//
//void LocalDeliver (std::string context, const Ipv6Header &ipHeader, Ptr<const Packet> ipPayload, uint32_t interface)
//{
//  NS_LOG_UNCOND (context << " " << ipHeader << " " << interface);
//  Ipv6FlowProbeTag fTag;
//  bool found = ipPayload->PeekPacketTag (fTag);
//  if (found)
//    NS_LOG_UNCOND (context << " Tag found : " << fTag.GetFlowId () << " " << fTag.GetPacketId () << " " << fTag.GetPacketSize ());
//}
//
//void Rx (std::string context, Ptr<const Packet> packet, Ptr<Ipv6> ipv6, uint32_t interface)
//{
////  NS_LOG_UNCOND ("Packet Tags");
////  packet->PrintPacketTags (std::cout); std::cout << std::endl;
//  Ipv6Header ipv6Header;
//  packet->PeekHeader (ipv6Header);
//  NS_LOG_UNCOND (context << " " << ipv6Header << " " << interface);
//  Ipv6FlowProbeTag fTag;
//  bool found = packet->PeekPacketTag (fTag);
//  if (found)
//    NS_LOG_UNCOND (context << " Tag found : " << fTag.GetFlowId () << " " << fTag.GetPacketId () << " " << fTag.GetPacketSize ());
//}
//
//void Tx (std::string context, Ptr<const Packet> packet, Ptr<Ipv6> ipv6, uint32_t interface)
//{
////  NS_LOG_UNCOND ("Packet Tags");
////  packet->PrintPacketTags (std::cout); std::cout << std::endl;
//  Ipv6Header ipv6Header;
//  packet->PeekHeader (ipv6Header);
//  NS_LOG_UNCOND (context << " " << ipv6Header << " " << interface);
//  Ipv6FlowProbeTag fTag;
//  bool found = packet->PeekPacketTag (fTag);
//  if (found)
//    NS_LOG_UNCOND (context << " Tag found : " << fTag.GetFlowId () << " " << fTag.GetPacketId () << " " << fTag.GetPacketSize ());
//}

int
main (int argc, char *argv[])
{
//  Config::SetDefault ("ns3::Ipv6::IpForward", BooleanValue (true));
  uint16_t numberOfNodes = 2;
  uint32_t maxPackets = 1;
  double simTime = 2;
  double distance = 60.0;
  double interPacketInterval = 100;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  cmd.AddValue ("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue ("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue ("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  cmd.AddValue ("maxPackets", "The maximum number of packets to be sent by the application", maxPackets);
  cmd.Parse(argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpc6Helper> epcHelper = CreateObject<PointToPointEpc6Helper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  p2ph.EnablePcap ("internet", internetDevices, true);

  Ipv6AddressHelper ipv6h;
  ipv6h.SetBase ("c0::", "64");
  Ipv6InterfaceContainer internetIpIfaces = ipv6h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv6Address remoteHostAddr = internetIpIfaces.GetAddress (1, 1);
  Ipv6Address pgwInternetAddr = internetIpIfaces.GetAddress (0, 1);

  Ipv6StaticRoutingHelper ipv6RoutingHelper;
  Ptr<Ipv6StaticRouting> remoteHostStaticRouting = ipv6RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv6> ());
  remoteHostStaticRouting->AddNetworkRouteTo ("b0::", 64, pgwInternetAddr, 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      enbPositionAlloc->Add (Vector(distance * i, 0, 0));
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(enbPositionAlloc);
  mobility.Install(enbNodes);
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      uePositionAlloc->Add (Vector(distance * i, 20, 0));
    }
  mobility.SetPositionAllocator(uePositionAlloc);
  mobility.Install(ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  for (uint32_t i = 0; i < ueLteDevs.GetN (); i++)
    ueLteDevs.Get (i)->SetAddress (Mac48Address::Allocate ());
  Ipv6InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv6StaticRouting> ueStaticRouting = ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Print PGW/SGW info
  NS_LOG_INFO ("PGW/SGW node");
  PrintCompleteNodeInfo (epcHelper->GetPgwNode ());

  // Print EnB Info
  for (uint32_t i = 0; i < enbNodes.GetN (); i++)
    {
      NS_LOG_INFO ("EnB " << i);
      PrintCompleteNodeInfo (enbNodes.Get (i));
    }

  // Print UE Info
  for (uint32_t i = 0; i < ueNodes.GetN (); i++)
    {
      NS_LOG_INFO ("UE " << i);
      PrintCompleteNodeInfo (ueNodes.Get (i));
    }

  // Print remote host info
  NS_LOG_INFO ("Remote host");
  PrintCompleteNodeInfo (remoteHostContainer.Get (0));


  // Attach one UE per eNodeB
  for (uint16_t i = 0; i < numberOfNodes; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i), false);
        // side effect: the default EPS bearer will be activated
      }

  // Add traces to sgw/pgw node.
  Config::Connect ("/NodeList/*/$ns3::Ipv6L3Protocol/Rx", MakeCallback (&RxTrace));
  Config::Connect ("/NodeList/*/$ns3::Ipv6L3Protocol/Tx", MakeCallback (&TxTrace));
  Config::Connect ("/NodeList/*/$ns3::Ipv6L3Protocol/Drop", MakeCallback (&DropTrace));

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      ++ulPort;
      ++otherPort;
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny (), otherPort));
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u, 1), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
      dlClient.SetAttribute ("PacketSize", UintegerValue (100));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue(maxPackets));

      UdpClientHelper client (ueIpIface.GetAddress (u, 1), otherPort);
      client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      client.SetAttribute ("MaxPackets", UintegerValue(maxPackets));

      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));
      if (u+1 < ueNodes.GetN ())
        {
          clientApps.Add (client.Install (ueNodes.Get(u+1)));
        }
      else
        {
          clientApps.Add (client.Install (ueNodes.Get(0)));
        }
    }
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&PacketSinkRxTrace));
  serverApps.Start (Seconds (1));
  clientApps.Start (Seconds (1));
  lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-epc-first");
//  NodeContainer flowMonitorNodes;
//  flowMonitorNodes.Add (ueNodes);
//  flowMonitorNodes.Add (enbNodes);
//  flowMonitorNodes.Add (pgw);
//  flowMonitorNodes.Add (remoteHostContainer);
//  FlowMonitorHelper flowmon;
//  Ptr<FlowMonitor> monitor = flowmon.Install (flowMonitorNodes);
//  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

//  NodeContainer attachL3Traces;
//  attachL3Traces.Add (enbNodes);
//  attachL3Traces.Add (pgw);
//  attachL3Traces.Add (ueNodes);
//  for (NodeContainer::Iterator it = attachL3Traces.Begin (); it != attachL3Traces.End (); it++)
//    {
//      Ptr<Node> n = *it;
//      std::ostringstream oss;
//      oss << "/NodeList/"  << n->GetId () << "/$ns3::Ipv6L3Protocol/";
//      Config::Connect (oss.str () + "SendOutgoing", MakeCallback (&SendOutgoing));
//      Config::Connect (oss.str () + "/LocalDeliver", MakeCallback (&LocalDeliver));
//      Config::Connect (oss.str () + "UnicastForward", MakeCallback (&UnicastForward));
//      Config::Connect (oss.str () + "Rx", MakeCallback (&Rx));
//      Config::Connect (oss.str () + "Tx", MakeCallback (&Tx));
//    }


  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

//  monitor->SerializeToXmlStream (std::cout, 2, false, false);
  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy();
  return 0;

}

