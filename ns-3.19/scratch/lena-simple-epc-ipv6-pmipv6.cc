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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/pmipv6-module.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */
NS_LOG_COMPONENT_DEFINE ("LenaPmipv6");

void PrintCompleteNodeInfo(Ptr<Node> node)
{
  int n_interfaces, n_ipaddrs, i, j;
  Ptr<Ipv6> ipv6;
  Ptr<Ipv6L3Protocol> ipv6l3;
  Ipv6InterfaceAddress ipv6address;

  ipv6 = node->GetObject<Ipv6> ();
  ipv6l3 = node->GetObject<Ipv6L3Protocol> ();
  n_interfaces = ipv6->GetNInterfaces();
  NS_LOG_UNCOND ("No of interfaces: " << n_interfaces);
  for (i = 0; i < n_interfaces; i++)
  {
    n_ipaddrs = ipv6->GetNAddresses(i);
    NS_LOG_UNCOND ("Interface " << i << " Forwarding: " << ipv6l3->GetInterface (i)->IsForwarding ());
    for (j = 0; j < n_ipaddrs; j++)
    {
      ipv6address = ipv6->GetAddress(i, j);
      NS_LOG_UNCOND (ipv6address);
    }
  }
  OutputStreamWrapper osw = OutputStreamWrapper (&std::cout);
  Ptr<Ipv6RoutingProtocol> ipv6rp = ipv6->GetRoutingProtocol();
  ipv6rp->PrintRoutingTable(&osw);
}

void RxTrace (std::string context, Ptr<const Packet> packet, Ptr<Ipv6> ipv6, uint32_t interfaceId)
{
  NS_LOG_DEBUG (context << " " << interfaceId);
}

void TxTrace (std::string context, Ptr<const Packet> packet, Ptr<Ipv6> ipv6, uint32_t interfaceId)
{
  Ipv6Header ipv6Header;
  packet->PeekHeader (ipv6Header);
  NS_LOG_DEBUG (context << " " << ipv6Header << " " << interfaceId);
}

void DropTrace (std::string context, const Ipv6Header & ipv6Header, Ptr<const Packet> packet, Ipv6L3Protocol::DropReason dropReason, Ptr<Ipv6> ipv6, uint32_t interfaceId)
{
  NS_LOG_DEBUG (context << " " << ipv6Header.GetSourceAddress () << " " << ipv6Header.GetDestinationAddress () << " " << dropReason << " " << interfaceId);
}

void PacketSinkRxTrace (std::string context, Ptr<const Packet> packet, const Address &address)
{
  SeqTsHeader seqTs;
  packet->Copy ()->RemoveHeader (seqTs);
  NS_LOG_UNCOND (context << " " << seqTs.GetTs () << "->" << Simulator::Now() << ": " << seqTs.GetSeq());
}

struct Args
{
  NodeContainer ueNodes;
  Ptr<PointToPointEpc6Pmipv6Helper> epcHelper;
  Ptr<Node> remoteHost;
  Ipv6InterfaceContainer ueIpIface;
  double interPacketInterval;
  uint32_t maxPackets;
  Ipv6Address remoteHostAddr;
};

void InstallApplications (Args args)
{
  NS_LOG_UNCOND ("Installing Applications");
  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < args.ueNodes.GetN (); ++u)
    {
      ++ulPort;
      ++otherPort;
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", Inet6SocketAddress (Ipv6Address::GetAny (), otherPort));
      serverApps.Add (dlPacketSinkHelper.Install (args.ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (args.remoteHost));
      serverApps.Add (packetSinkHelper.Install (args.ueNodes.Get(u)));

      UdpClientHelper dlClient (args.ueIpIface.GetAddress (u, 1), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(args.interPacketInterval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue (args.maxPackets));
      dlClient.SetAttribute ("PacketSize", UintegerValue (100));

      UdpClientHelper ulClient (args.remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(args.interPacketInterval)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue(args.maxPackets));
      ulClient.SetAttribute ("PacketSize", UintegerValue (100));

      UdpClientHelper client (args.ueIpIface.GetAddress (u, 1), otherPort);
      client.SetAttribute ("Interval", TimeValue (MilliSeconds(args.interPacketInterval)));
      client.SetAttribute ("MaxPackets", UintegerValue(args.maxPackets));
      client.SetAttribute ("PacketSize", UintegerValue (100));

      clientApps.Add (dlClient.Install (args.remoteHost));
      clientApps.Add (ulClient.Install (args.ueNodes.Get(u)));
      if (u+1 < args.ueNodes.GetN ())
        {
          clientApps.Add (client.Install (args.ueNodes.Get(u+1)));
        }
      else
        {
          clientApps.Add (client.Install (args.ueNodes.Get(0)));
        }
    }
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&PacketSinkRxTrace));
  serverApps.Start (Seconds (1));
  clientApps.Start (Seconds (1));
}

void PrintLteNodesInfo (Ptr<PointToPointEpc6Pmipv6Helper> epcHelper, NodeContainer enbNodes, NodeContainer ueNodes)
{
  // Print PGW info
  NS_LOG_UNCOND ("PGW node");
  PrintCompleteNodeInfo (epcHelper->GetPgwNode ());

  // Print SGW info
  NS_LOG_UNCOND ("SGW node");
  PrintCompleteNodeInfo (epcHelper->GetSgwNode ());

  // Print EnB Info
  for (uint32_t i = 0; i < enbNodes.GetN (); i++)
    {
      NS_LOG_UNCOND ("EnB " << i);
      PrintCompleteNodeInfo (enbNodes.Get (i));
    }

  // Print UE Info
  for (uint32_t i = 0; i < ueNodes.GetN (); i++)
    {
      NS_LOG_UNCOND ("UE " << i);
      PrintCompleteNodeInfo (ueNodes.Get (i));
    }
}

int
main (int argc, char *argv[])
{
  uint16_t numberOfNodes = 2;
  uint32_t maxPackets = 1;
  double simTime = 15;
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

  Mac48Address magMacAddress ("00:00:aa:bb:cc:dd");
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpc6Pmipv6Helper> epcHelper = CreateObject<PointToPointEpc6Pmipv6Helper> (magMacAddress);
  lteHelper->SetEpcHelper (epcHelper);
  epcHelper->SetupS5Interface ();
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Stop DAD on remote host as DAD is disabled on PGW as well.
  Ptr<Icmpv6L4Protocol> remoteHostIcmpv6L4Protocol = remoteHost->GetObject<Icmpv6L4Protocol> ();
  remoteHostIcmpv6L4Protocol->SetAttribute ("DAD", BooleanValue (false));

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (300)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv6AddressHelper ipv6h;
  ipv6h.SetBase ("c0::", "64");
  Ipv6InterfaceContainer internetIpIfaces = ipv6h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv6Address remoteHostAddr = internetIpIfaces.GetAddress (1, 1);
  Ipv6Address pgwInternetAddr = internetIpIfaces.GetAddress (0, 1);

  // Create Static route for Remote Host to reach the UEs.
  Ipv6StaticRoutingHelper ipv6RoutingHelper;
  Ptr<Ipv6StaticRouting> remoteHostStaticRouting = ipv6RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv6> ());
  remoteHostStaticRouting->AddNetworkRouteTo ("b0::", 32, pgwInternetAddr, 1);

  // Create eNB and UE nodes.
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (numberOfNodes);
  ueNodes.Create (numberOfNodes);

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

  // Install LTE Devices to the UE and eNB nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs and enable the interfaces to have link-local Ipv6 addresses.
  internet.Install (ueNodes);
  for (uint32_t i = 0; i < ueLteDevs.GetN (); i++)
    ueLteDevs.Get (i)->SetAddress (Mac48Address::Allocate ());
  Ipv6InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignWithoutAddress (ueLteDevs);

  // Attach one UE per eNB
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i), false);
      // side effect: the default EPS bearer will be activated
    }

  // Add PMIPv6 profiles.
  Ptr<Pmipv6ProfileHelper> profile = epcHelper->GetPmipv6ProfileHelper ();

  // Add profile for each UE
  std::ostringstream oss;
  for (uint32_t i = 0; i < ueLteDevs.GetN (); i++)
    {
      oss.str ("");
      oss.clear ();
      oss << "node" << i << "@iith.ac.in";
      Ptr<NetDevice> netDev = ueLteDevs.Get (i);
      Ptr<LteUeNetDevice> ueLteNetDev = DynamicCast<LteUeNetDevice> (netDev);
      profile->AddProfile (Identifier (oss.str ().c_str ()), Identifier (), pgwInternetAddr, std::list<Ipv6Address> (), ueLteNetDev->GetImsi ());
    }

  // Enable PCAP traces
  p2ph.EnablePcapAll ("lte-pmipv6");
  epcHelper->EnablePcap ("lte-pmipv6", ueLteDevs);
  epcHelper->EnablePcap ("lte-pmipv6", enbLteDevs);

  // Add IP traces to all nodes.
  Config::Connect ("/NodeList/*/$ns3::Ipv6L3Protocol/Rx", MakeCallback (&RxTrace));
  Config::Connect ("/NodeList/*/$ns3::Ipv6L3Protocol/Tx", MakeCallback (&TxTrace));
  Config::Connect ("/NodeList/*/$ns3::Ipv6L3Protocol/Drop", MakeCallback (&DropTrace));

  // Schedule Applications.
  Args args;
  args.ueNodes = ueNodes;
  args.epcHelper = epcHelper;
  args.remoteHost = remoteHost;
  args.ueIpIface = ueIpIface;
  args.interPacketInterval = interPacketInterval;
  args.maxPackets = maxPackets;
  args.remoteHostAddr = remoteHostAddr;
  Simulator::Schedule (Seconds (10), &InstallApplications, args);

  // Print Information
  PrintLteNodesInfo (epcHelper, enbNodes, ueNodes);
  // Print remote host info
  NS_LOG_UNCOND ("Remote host");
  PrintCompleteNodeInfo (remoteHostContainer.Get (0));
  // Schedule print information
  Simulator::Schedule (Seconds (6), &PrintLteNodesInfo, epcHelper, enbNodes, ueNodes);

  // Run simulation
  Simulator::Stop(Seconds(simTime));
  Simulator::Run();
  Simulator::Destroy();
  return 0;

}

