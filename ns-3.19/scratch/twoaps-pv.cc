/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h" // for flowmonitor
#include "ns3/packet-sink.h"  // for tcp bulk send application
#include  <string.h>
#include "ns3/netanim-module.h"
#include "ns3/bridge-helper.h"
#include <stdio.h>

// Network Topology ( 2 APs connected to a server and one Wifi Station node n3)
//
//Wifi 10.1.0.1
//       AP1
//  *    *
//  |    |   10.1.1.0
// n3   n1 -------------- 
//         point-to-point |
//10.1.0.3
//
//Wifi 10.1.0.2
//       AP2             n0 (server)
// *     *
// |     |   10.1.2.0     |
// n3    n2  --------------  
//         point-to-point 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");

int main (int argc, char *argv[])
{
  
  uint32_t maxBytes = 0;
  uint32_t  N = 3;
  uint32_t maxRange = 50;
  uint32_t maxspeed=20;
  uint32_t simtime=300;
  uint32_t i=0;
  CommandLine cmd;
  cmd.AddValue ("N", "Number of wifi AP devices", N);
  cmd.AddValue ("speed", "Speed of wifi sta device", maxspeed);
  cmd.AddValue ("simtime", "Simulation time in seconds", simtime);
  cmd.Parse (argc,argv);

  /* ************************************************************************************************
  
                        CREATE POINT TO POINT LINK BETWEEN SERVER NODE AND THE 2 APS
                                
  *************************************************************************************************** */

 
  //Here, we will create nodes.
  NS_LOG_INFO ("Create nodes.");
  NodeContainer serverNode;
  NodeContainer APNodes;
  serverNode.Create (1);
  APNodes.Create (N-1);
  NodeContainer allNodes = NodeContainer (serverNode, APNodes);
 
  // Install network stacks on the nodes
  InternetStackHelper internet;
  internet.Install (allNodes);
 
  //Collect an adjacency list of nodes for the p2p topology
  std::vector<NodeContainer> nodeAdjacencyList (N-1);
  for( i=0; i<nodeAdjacencyList.size (); ++i)
  {
  nodeAdjacencyList[i] = NodeContainer (serverNode, APNodes.Get (i));
  }
 
  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("0ms"));
  std::vector<NetDeviceContainer> deviceAdjacencyList (N-1);
  for( i=0; i<deviceAdjacencyList.size (); ++i)
  {
  deviceAdjacencyList[i] = p2p.Install (nodeAdjacencyList[i]);
  }
 
  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  std::vector<Ipv4InterfaceContainer> interfaceAdjacencyList (N-1);
  for( i=0; i<interfaceAdjacencyList.size (); ++i)
  {
  std::ostringstream subnet;
  subnet<<"10.1."<<i+1<<".0";
  ipv4.SetBase (subnet.str ().c_str (), "255.255.255.0");
  interfaceAdjacencyList[i] = ipv4.Assign (deviceAdjacencyList[i]);
  }
    
  
  /* *****************************************************************************************************************
  
                               TO CREATE THE WIFI TOPOLOGY WITH ONE WIFI STATION NODE AND 2 APS 
  
  ********************************************************************************************************************* */

  
  NodeContainer stas;
  stas.Create(1);
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel","MaxRange", DoubleValue (maxRange)); 
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0.0),
                                  "MinY", DoubleValue (0.0),
                                  "DeltaX", DoubleValue (80.0),
                                  "DeltaY", DoubleValue (80.0),
                                  "GridWidth", UintegerValue (3),
                                  "LayoutType", StringValue("RowFirst"));
 
 WifiHelper wifiInfra;
 NqosWifiMacHelper macInfra;
 Ssid ssid;

 ipv4.SetBase ("10.1.0.0", "255.255.255.0");
 for (uint32_t i = 0; i < N-1; ++i)
    {
      NS_LOG_INFO ("Configuring wireless network for AP node " << i);
      // Create an infrastructure network
      wifiInfra = WifiHelper::Default ();
      macInfra = NqosWifiMacHelper::Default ();
      wifiPhy.SetChannel (wifiChannel.Create ());
      // Create unique ssids for these networks
      std::string ssidString ("wifi-infra");
      std::stringstream ss;
      ss << i;
      ssidString += ss.str ();
      ssid = Ssid (ssidString);
      wifiInfra.SetRemoteStationManager ("ns3::ArfWifiManager");   
      // setup ap.
      macInfra.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid),  "BeaconGeneration", BooleanValue (true),
      "EnableBeaconJitter",BooleanValue (true),
      "BeaconInterval", TimeValue (MilliSeconds (100)));
      NetDeviceContainer apDevices = wifiInfra.Install (wifiPhy, macInfra, nodeAdjacencyList[i].Get(1));
      ipv4.Assign (apDevices);
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");  
      mobility.Install (nodeAdjacencyList[i].Get(1));
    }
    std::stringstream ss;
    ss <<"ns3::ConstantRandomVariable[Constant=-"<< maxspeed<<"]";
    puts(ss.str().c_str());                    
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                                 "Mode", StringValue ("Time"),
                                 "Time", StringValue ("300s"),
                                 "Speed", StringValue(ss.str().c_str()),
                                 "Bounds", StringValue ("-50|160|-50|50"));
                            
   mobility.Install (stas);
  // Add the IPv4 protocol stack to the nodes in our container
  internet.Install (stas); 
  macInfra.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
  NetDeviceContainer staDevices = wifiInfra.Install (wifiPhy, macInfra, stas);
  ipv4.Assign (staDevices);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  //Create a BulkSendApplication and install it on node 0

  uint16_t port = 9;  // well-known echo port number
  BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (interfaceAdjacencyList[1].GetAddress (0), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps = source.Install (stas.Get (0));
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));
  //
  // Create a PacketSinkApplication and install it on node 1
  //
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (serverNode.Get(0));
  sinkApps.Start (Seconds(2));
  sinkApps.Stop (Seconds(simtime));
  p2p.EnablePcapAll("third");

    
  //throughput calculation added
  FlowMonitorHelper flowmon;
  Ptr < FlowMonitor > monitor = flowmon.InstallAll();
  //
  // Now, do the actual simulation.
  //;
  Simulator::Stop (Seconds(simtime));
  AnimationInterface anim ("mat.xml");
  Simulator::Run ();
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i!= stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);


          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout <<"last time " <<i->second.timeLastRxPacket.GetSeconds()<<" first time"<<i->second.timeFirstTxPacket.GetSeconds()<<"\n";
            std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
    }    
  Simulator::Destroy ();
  return 0;
}
