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

// Creates a node which moves around within a bounding box. Every 10 seconds
// the node will change it's movement. After a certain time the bounding box
// is made bigger and time after which the node changes it's movement is
// changed to 15 seconds.

// Running instructions
// ./waf --run ns3-1
// ./waf --run ns3-1 --vis

#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include <cmath>
using namespace ns3;

static void
CourseChangeCallback (std::string path, Ptr<const MobilityModel> model)
{
  // Get position using a built Get method within the class.
  Vector position = model->GetPosition ();
  Vector3DValue velocityValue;
  // Get velocity using the attribute system.
  model->GetAttribute ("Velocity", velocityValue);
  std::cout << Simulator::Now().GetSeconds () << " seconds: CourseChange " << path << " Position = (x=" << position.x << ", y=" << position.y << ") "
    << "Velocity = (x=" << velocityValue.Get ().x << ", y=" << velocityValue.Get ().y << ") | " << sqrt (pow (velocityValue.Get ().x, 2) + pow (velocityValue.Get ().y, 2)) << std::endl;
}

static void
ChangeBounds (NodeContainer nodes)
{
  std::cout << "ChangeBounds called\n";
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<RandomWalk2dMobilityModel> mobilityModel = DynamicCast<RandomWalk2dMobilityModel> (nodes.Get (i)->GetObject<MobilityModel> ());
      if (mobilityModel != NULL)
        {
          // Change the bounding box to make it bigger.
          mobilityModel->SetAttribute ("Bounds", StringValue("-100|100|-100|100"));
        }
    }
}

static void
ChangeTime (NodeContainer nodes)
{
  std::cout << "ChangeTime called\n";
  for (uint32_t i = 0; i < nodes.GetN (); i++)
    {
      Ptr<RandomWalk2dMobilityModel> mobilityModel = DynamicCast<RandomWalk2dMobilityModel> (nodes.Get (i)->GetObject<MobilityModel> ());
      if (mobilityModel != NULL)
        {
          // Instead of 10s now the node will change movement every 15 seconds.
          mobilityModel->SetAttribute ("Time", StringValue("15"));
        }
    }
}

int 
main (int argc, char *argv[])
{
  uint32_t simTime = 200;
  uint32_t n = 1;
  CommandLine cmd;
  cmd.AddValue ("n", "Number of nodes", n);
  cmd.AddValue ("simTime", "Simulation time (in seconds)", simTime);
  cmd.Parse (argc,argv);

  NodeContainer nodes;
  nodes.Create (n);
  MobilityHelper mobility;
  // Assign initial positions to the nodes.
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  // The nodes initially move within a bounding box with coordinates (-50, -50) and (50, 50). Every 10 seconds the node will change it's movement.
  // If the node reaches a boundary it will change it's direction before the the time is up.
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)),
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ("10s"),
                             "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=3]"));
  mobility.Install (nodes);

  // Log whenever the node changes it's movement.
  Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback (&CourseChangeCallback));

  // Schedule couple of changes in the mobility parameters.
  Simulator::Schedule (Seconds (100), &ChangeBounds, nodes);
  Simulator::Schedule (Seconds (120), &ChangeTime, nodes);

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
