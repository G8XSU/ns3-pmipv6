#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/bridge-helper.h"
#include <vector>
#include <stdint.h>
#include <sstream>
#include <fstream>

using namespace ns3;

static void
SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}

static void
SetVelocity (Ptr<Node> node, Vector velocity)
{
  Ptr<ConstantVelocityMobilityModel> mobility = node->GetObject<ConstantVelocityMobilityModel> ();
  mobility->SetVelocity (velocity);
}

static void udpRx (std::string context, Ptr<const Packet> packet, const Address &address) {
  SeqTsHeader seqTs;
  packet->Copy ()->RemoveHeader (seqTs);
  std::cout << seqTs.GetTs () << "->" << Simulator::Now() << ": " << seqTs.GetSeq() << std::endl;
//  NS_LOG_INFO(seqTs.GetTs () << "->" << Simulator::Now() << ": " << seqTs.GetSeq());
}

int main (int argc, char **argv)
{
  uint32_t nWifis = 2;
  uint32_t nStas = 1;
  bool enablePcap = false;

  CommandLine cmd;
  cmd.AddValue ("nWifis", "Number of wifi networks", nWifis);
  cmd.AddValue ("nStas", "Number of stations per wifi network", nStas);
  cmd.AddValue ("enablePcap", "Enable pcap traces.", enablePcap);
  cmd.Parse (argc, argv);

  NodeContainer backboneNodes;
  NetDeviceContainer backboneDevices;
  Ipv4InterfaceContainer backboneInterfaces;
  std::vector<NetDeviceContainer> apDevices;
  std::vector<Ipv4InterfaceContainer> apInterfaces;

  InternetStackHelper stack;
  CsmaHelper csma;
  Ipv4AddressHelper ip;
  ip.SetBase ("192.168.0.0", "255.255.255.0");

  backboneNodes.Create (nWifis + 1);
  stack.Install (backboneNodes);

  backboneDevices = csma.Install (backboneNodes);
  Ipv4InterfaceContainer cnInterface = ip.Assign (backboneDevices.Get (nWifis));

  Ssid ssid = Ssid ("wifi-wired");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
  WifiHelper wifi = WifiHelper::Default ();
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  MobilityHelper mobility;
  int wifiX = 0;
  for (uint32_t i = 0; i < nWifis; ++i)
  {
    NetDeviceContainer apDev;
    Ipv4InterfaceContainer apInterface;
    BridgeHelper bridge;

    // setup the AP.
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (backboneNodes.Get (i));
    SetPosition (backboneNodes.Get (i), Vector (wifiX, 0, 0));
    wifiMac.SetType ("ns3::ApWifiMac",
                     "Ssid", SsidValue (ssid));
    apDev = wifi.Install (wifiPhy, wifiMac, backboneNodes.Get (i));

    NetDeviceContainer bridgeDev;
    bridgeDev = bridge.Install (backboneNodes.Get (i), NetDeviceContainer (apDev, backboneDevices.Get (i)));

    // assign AP IP address to bridge, not wifi
    apInterface = ip.Assign (bridgeDev);

    // save everything in containers.
    apDevices.push_back (apDev);
    apInterfaces.push_back (apInterface);
    wifiX += 100.0;
  }

  NodeContainer sta;
  NetDeviceContainer staDev;
  Ipv4InterfaceContainer staInterface;
  sta.Create (nStas);
  // setup the STAs
  stack.Install (sta);
//  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
//                             "Mode", StringValue ("Time"),
//                             "Time", StringValue ("10s"),
//                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
//                             "Bounds", RectangleValue (Rectangle (0, wifiX + 100, -10, 10)));
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (sta);
  SetPosition (sta.Get (0), Vector (0, 5, 0));
  SetVelocity (sta.Get (0), Vector (5, 0, 0));
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));
  staDev = wifi.Install (wifiPhy, wifiMac, sta);
  staInterface = ip.Assign (staDev);

  uint32_t port = 6000;
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer serverApps = sink.Install (backboneNodes.Get (nWifis));

  UdpClientHelper udpClient(Ipv4Address("192.168.0.1"), port);
  udpClient.SetAttribute ("Interval", TimeValue (MilliSeconds (100)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));
  udpClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  ApplicationContainer clientApps = udpClient.Install (sta.Get (0));
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback(&udpRx));
  serverApps.Start (Seconds (1.0));
  clientApps.Start (Seconds (1.0));
  if (enablePcap)
    {
      csma.EnablePcap ("wifi-wired1", backboneNodes);
      wifiPhy.EnablePcap ("wifi-wired1", sta);
      for (uint32_t i = 0; i < nWifis; i++)
        wifiPhy.EnablePcap ("wifi-wired1", NodeContainer (backboneNodes.Get (i)));
    }

  Simulator::Stop (Seconds (200));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
