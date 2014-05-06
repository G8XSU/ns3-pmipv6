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

#include <stdio.h>
#include <sstream>

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-mac.h"
#include "ns3/regular-wifi-mac.h"
//#include "ns3/wimax-net-device.h"
#include "ns3/epc6-sgw-application.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/ipv6-static-source-routing-helper.h"

#include "ipv6-static-source-routing.h"
#include "ipv6-mobility-header.h"
#include "ipv6-mobility.h"
#include "ipv6-mobility-demux.h"
#include "ipv6-mobility-option-header.h"
#include "ipv6-mobility-option.h"
#include "ipv6-mobility-l4-protocol.h"
#include "ipv6-tunnel-l4-protocol.h"
#include "unicast-radvd.h"
#include "lte-unicast-radvd.h"
#include "regular-unicast-radvd.h"
#include "pmipv6-profile.h"
#include "pmipv6-mag-notifier.h"
#include "pmipv6-mag.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("Pmipv6Mag");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Pmipv6Mag);

Pmipv6Mag::Pmipv6Mag ()
: m_useRemoteAp (false),
  m_isLteMag (false),
  m_sequence (0),
  m_buList (0),
  m_radvd (0),
  m_ifIndex (-1)
{
}

Pmipv6Mag::~Pmipv6Mag ()
{
  m_buList = 0;
  m_radvd = 0;
}

void Pmipv6Mag::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_buList->Flush ();
  m_buList = 0;
  m_radvd = 0;
  Pmipv6Agent::DoDispose ();
}

void Pmipv6Mag::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (GetNode () == 0)
    {
      Ptr<Node> node = this->GetObject<Node> ();
      SetNode (node);
      m_buList = CreateObject<BindingUpdateList> ();
      m_buList->SetNode (node);

      if (m_isLteMag)
        {
          Ptr<Epc6SgwApplication> epc6SgwApp;
          for (uint32_t i = 0; i < node->GetNApplications (); i++)
            {
              Ptr<Application> app = node->GetApplication (i);
              epc6SgwApp = DynamicCast<Epc6SgwApplication> (app);
              if (epc6SgwApp)
                {
                  NS_LOG_INFO ("Installed callback " << i);
                  epc6SgwApp->SetUePrefixCallback (m_setIp);
                  epc6SgwApp->SetNewHostCallback (MakeCallback (&Pmipv6Mag::HandleLteNewNode, this));
                  m_ifIndex = (int16_t) epc6SgwApp->GetTunnelInterfaceId ();
                  break;
                }
            }
          Ptr<LteUnicastRadvd> unicastRadvd = CreateObject<LteUnicastRadvd> ();
          unicastRadvd->SetStartTime (Seconds (1));
          node->AddApplication (unicastRadvd);
          unicastRadvd->SetSendCallback ();
          m_radvd = DynamicCast<UnicastRadvd> (unicastRadvd);
        }
      else
        {
          if (!m_useRemoteAp)
            {
              // register linkup callback.
              uint32_t nDev = node->GetNDevices ();
              for (uint32_t i = 0; i < nDev; ++i)
                {
                  Ptr<NetDevice> dev = node->GetDevice (i);
                  Ptr<WifiNetDevice> wDev = dev->GetObject<WifiNetDevice> ();
                  if (wDev)
                    {
                      Ptr<WifiMac> mac = wDev->GetMac ();
                      Ptr<RegularWifiMac> rmac = mac->GetObject<RegularWifiMac> ();
                      if (!mac || !rmac)
                        {
                          continue;
                        }
                      rmac->SetNewHostCallback (MakeCallback (&Pmipv6Mag::HandleRegularNewNode, this));
                    }
    //              Ptr<WimaxNetDevice> wDev2 = dev->GetObject<WimaxNetDevice> ();
    //              if (wDev2)
    //                {
    //                  wDev2->SetNewHostCallback (MakeCallback (&Pmipv6Mag::HandleNewNode, this));
    //                  continue;
    //                }
                }
            }
          else
            {
              Ptr<Pmipv6MagNotifier> noti = GetNode ()->GetObject<Pmipv6MagNotifier> ();
              NS_ASSERT (noti != 0);
              noti->SetNewNodeCallback (MakeCallback (&Pmipv6Mag::HandleRegularNewNode, this));
            }
          // RADVD Setting
          Ptr<RegularUnicastRadvd> unicastRadvd = CreateObject<RegularUnicastRadvd> ();
          unicastRadvd->SetStartTime (Seconds (1.));
          node->AddApplication (unicastRadvd);
          m_radvd = DynamicCast<UnicastRadvd> (unicastRadvd);
        }
    }
  Pmipv6Agent::NotifyNewAggregate ();
}

bool Pmipv6Mag::IsUseRemoteAP () const
{
  return m_useRemoteAp;
}

void Pmipv6Mag::UseRemoteAP (bool remoteAp)
{
  m_useRemoteAp = remoteAp;
}

bool Pmipv6Mag::IsLteMag () const
{
  return m_isLteMag;
}

void Pmipv6Mag::SetLteMag (bool isLteMag)
{
  m_isLteMag = isLteMag;
}

Ipv6Address Pmipv6Mag::GetLinkLocalAddress (Ipv6Address addr)
{
  NS_LOG_FUNCTION (this << addr);

  Ipv6Address lla;
  Ptr<Ipv6L3Protocol> ipv6 = GetNode ()->GetObject<Ipv6L3Protocol> ();
  NS_ASSERT (ipv6 != 0 && ipv6->GetRoutingProtocol () != 0);

  Ptr<Packet> p = Create<Packet> ();

  Ipv6Header header;
  SocketIpTtlTag tag;
  Socket::SocketErrno err;
  Ptr<Ipv6Route> route;
  Ptr<NetDevice> oif (0); // specify non-zero if bound to a source address

  header.SetDestinationAddress (addr);
  route = ipv6->GetRoutingProtocol ()->RouteOutput (p, header, oif, err);
  if (route != 0)
    {
      Ptr<NetDevice> device = route->GetOutputDevice ();
      Ptr<Ipv6Interface> iif = ipv6->GetInterface (ipv6->GetInterfaceForDevice (device));
      Ipv6InterfaceAddress iia = iif->GetLinkLocalAddress ();
      lla = iia.GetAddress ();
      NS_LOG_LOGIC ("MAG's outgoing interface LLA is " << lla);
    }
  return lla;
}

uint16_t Pmipv6Mag::GetSequence ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return ++m_sequence;
}

Ptr<UnicastRadvd> Pmipv6Mag::GetRadvd () const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_radvd;
}

Ptr<Packet> Pmipv6Mag::BuildPbu (BindingUpdateList::Entry *bule)
{
  NS_LOG_FUNCTION(this << bule);

  Ptr<Packet> p = Create<Packet> ();

  Ipv6MobilityBindingUpdateHeader pbu;

  Ipv6MobilityOptionMobileNodeIdentifierHeader mnidh;
  Ipv6MobilityOptionHomeNetworkPrefixHeader hnph;
  Ipv6MobilityOptionHandoffIndicatorHeader hih;
  Ipv6MobilityOptionAccessTechnologyTypeHeader atth;
  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader mnllidh;
  Ipv6MobilityOptionTimestampHeader timestamph;

  pbu.SetSequence (bule->GetLastBindingUpdateSequence ());
  pbu.SetFlagA (true);
  pbu.SetFlagH (true);
  pbu.SetFlagL (true);
  pbu.SetFlagP (true);
  pbu.SetFlagT(false);
  pbu.SetLifetime ((uint16_t) Ipv6MobilityL4Protocol::MAX_BINDING_LIFETIME);

  // Add Mobile Node Identifier Option
  mnidh.SetSubtype (1);
  mnidh.SetNodeIdentifier (bule->GetMnIdentifier ());
  pbu.AddOption (mnidh);

  // Add Home Network Prerfix List (if it has)
  if (bule->GetHomeNetworkPrefixes ().size () > 0)
    {
      std::list<Ipv6Address> hnp = bule->GetHomeNetworkPrefixes ();
      for (std::list<Ipv6Address>::iterator i = hnp.begin (); i != hnp.end (); i++)
        {
          hnph.SetPrefix ((*i));
          hnph.SetPrefixLength (64);
          pbu.AddOption (hnph);
        }
    }
  else
    {
      hnph.SetPrefix (Ipv6Address::GetAny ());
      hnph.SetPrefixLength (0);
      pbu.AddOption (hnph);
    }

  // Add Handoff Indicator Option
  hih.SetHandoffIndicator (bule->GetHandoffIndicator ());
  pbu.AddOption (hih);

  // Add Access Technology Type Option
  atth.SetAccessTechnologyType (bule->GetAccessTechnologyType ());
  pbu.AddOption (atth);

  // Add MobileNode Link Identifier Option
  if (!bule->GetMnLinkIdentifier ().IsEmpty ())
    {
      mnllidh.SetLinkLayerIdentifier (bule->GetMnLinkIdentifier ());
      pbu.AddOption (mnllidh);
    }

  // Add Link Local Address Option (if available).
  if (!bule->GetMagLinkAddress ().IsAny ())
    {
      Ipv6MobilityOptionLinkLocalAddressHeader llah;
      llah.SetLinkLocalAddress (bule->GetMagLinkAddress ());
      pbu.AddOption (llah);
    }

  // Add Timestamp Option
  timestamph.SetTimestamp (bule->GetLastBindingUpdateTime ());
  pbu.AddOption (timestamph);

  p->AddHeader(pbu);

  return p;
}
Ptr<Packet> Pmipv6Mag::BuildHua(BindingUpdateList::Entry *bule, std::list<Ipv6Address> new_hnps)
{
  NS_LOG_FUNCTION("BuildHua" << bule);

  Ptr<Packet> p = Create<Packet> ();

  Ipv6MobilityBindingUpdateHeader hua;

  Ipv6MobilityOptionMobileNodeIdentifierHeader mnidh;
  Ipv6MobilityOptionHomeNetworkPrefixHeader hnph;
  Ipv6MobilityOptionHandoffIndicatorHeader hih;
  Ipv6MobilityOptionAccessTechnologyTypeHeader atth;
  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader mnllidh;
  Ipv6MobilityOptionTimestampHeader timestamph;

  hua.SetSequence (bule->GetLastBindingUpdateSequence ());
  hua.SetFlagA (true);
  hua.SetFlagH (true);
  hua.SetFlagL (true);
  hua.SetFlagP (true);
  hua.SetFlagT (true);
  hua.SetLifetime ((uint16_t) Ipv6MobilityL4Protocol::MAX_BINDING_LIFETIME);

  // Add Mobile Node Identifier Option
  mnidh.SetSubtype (1);
  mnidh.SetNodeIdentifier (bule->GetMnIdentifier ());
  hua.AddOption (mnidh);

  // Add Home Network Prerfix List (if it has)
  if (new_hnps.size () > 0)
	{
	  for (std::list<Ipv6Address>::iterator i = new_hnps.begin (); i != new_hnps.end (); i++)
		{
		  hnph.SetPrefix ((*i));
		  hnph.SetPrefixLength (64);
		  hua.AddOption (hnph);
		}
	}
  else
	{
	  hnph.SetPrefix (Ipv6Address::GetAny ());
	  hnph.SetPrefixLength (0);
	  hua.AddOption (hnph);
	}

  // Add Handoff Indicator Option
  hih.SetHandoffIndicator (bule->GetHandoffIndicator ());
  hua.AddOption (hih);

  // Add Access Technology Type Option
  atth.SetAccessTechnologyType (bule->GetAccessTechnologyType ());
  hua.AddOption (atth);

  // Add MobileNode Link Identifier Option
  if (!bule->GetMnLinkIdentifier ().IsEmpty ())
	{
	  mnllidh.SetLinkLayerIdentifier (bule->GetMnLinkIdentifier ());
	  hua.AddOption (mnllidh);
	}

  // Add Link Local Address Option (if available).
  if (!bule->GetMagLinkAddress ().IsAny ())
	{
	  Ipv6MobilityOptionLinkLocalAddressHeader llah;
	  llah.SetLinkLocalAddress (bule->GetMagLinkAddress ());
	  hua.AddOption (llah);
	}

  // Add Timestamp Option
  timestamph.SetTimestamp (bule->GetLastBindingUpdateTime ());
  hua.AddOption (timestamph);

  p->AddHeader(hua);

  return p;
}


void Pmipv6Mag::HandleRegularNewNode (Mac48Address from, Mac48Address to, uint8_t att)
{
  NS_LOG_FUNCTION (this << from << to <<(uint32_t)att);
  NS_ASSERT ( GetProfile() != 0 );

  // Get Profile
  Pmipv6Profile::Entry *pf = GetProfile ()->LookupMnLinkId (Identifier (from));
  if (pf == 0)
    {
      NS_LOG_LOGIC ("No profile exists for MAC(" << from << ") ATT(" << (uint32_t) att << ")");
      return;
    }

  // Check BUL
  BindingUpdateList::Entry *bule = m_buList->Lookup (pf->GetMnIdentifier ());
  if (bule == 0)
    {
      bule = m_buList->Add (pf->GetMnIdentifier ());
    }

  bule->SetAccessTechnologyType (att);
  bule->SetMnLinkIdentifier (pf->GetMnLinkIdentifier ());
  bule->SetLmaAddress (pf->GetLmaAddress ());
  if (pf->GetHomeNetworkPrefixes ().size () > 0)
    {
      bule->SetHomeNetworkPrefixes (pf->GetHomeNetworkPrefixes ());
    }

  //XXX: how to determine proper HI(Handoff Indicator) value??
  bule->SetHandoffIndicator (Ipv6MobilityHeader::OPT_NEW_ATTACHMENT); //OPT_HI_HANDOFF_STATE_UNKNOWN  //OPT_NEW_ATTACHMENT

  Ipv6Address lla = GetLinkLocalAddress (bule->GetLmaAddress ());
  if (!lla.IsAny ())
    {
      bule->SetMagLinkAddress (lla);
    }

  // Get IfIndex from "to"
  uint32_t nDev = GetNode ()->GetNDevices ();
  Ptr<NetDevice> dev = 0;
  bool found = false;
  for (uint32_t i = 0; i < nDev; ++i)
    {
      dev = GetNode ()->GetDevice (i);
      if (Mac48Address::ConvertFrom (dev->GetAddress ()) == to)
        {
          found = true;
          NS_LOG_LOGIC ("Found Device (" << dev->GetIfIndex () << ") for MAC address (" << to << ")");
          break;
        }
    }

  if (found == false)
    {
      NS_LOG_WARN ("Device Not Found for MAC address (" << to << ")");
      return;
    }

  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  NS_ASSERT (ipv6);

  int32_t ifIndex = ipv6->GetInterfaceForDevice (dev);
  if (ifIndex == -1)
    {
      NS_LOG_LOGIC ("No Ipv6Interface for Device " << dev->GetIfIndex ());
      return;
    }

  bule->SetIfIndex (ifIndex);

  // Preset header information
  bule->SetLastBindingUpdateSequence (GetSequence ());
  // Cut to micro-seconds
  bule->SetLastBindingUpdateTime (MicroSeconds (Simulator::Now ().GetMicroSeconds ()));

  Ptr<Packet> p = BuildPbu (bule);
  //save packet
  bule->SetPbuPacket (p);
  // Reset (for the first registration)
  bule->ResetRetryCount ();

  // send PBU
  SendMessage (p->Copy (), bule->GetLmaAddress (), 64);

  bule->StartRetransTimer ();
  if (bule->IsReachable ())
    {
      bule->MarkRefreshing ();
    }
  else
    {
      bule->MarkUpdating ();
    }
}

void
Pmipv6Mag::HandleLteNewNode (uint32_t teid, uint64_t imsi, uint8_t att)
{
  NS_LOG_FUNCTION (this << teid << imsi << (uint32_t) att);
  NS_ASSERT ( GetProfile() != 0 );

  // Get Profile
  Pmipv6Profile::Entry *pf = GetProfile ()->LookupImsi (imsi);
  if (pf == 0)
    {
      NS_LOG_LOGIC ("No profile exists for Imsi(" << imsi << ") Att(" << (uint32_t) att << ")");
      return;
    }

  // Check BUL
  BindingUpdateList::Entry *bule = m_buList->Lookup (pf->GetMnIdentifier ());
  if (bule == 0)
    {
      bule = m_buList->Add (pf->GetMnIdentifier ());
    }

  bule->SetAccessTechnologyType (att);
  bule->SetMnLinkIdentifier (Identifier ());
  bule->SetLmaAddress (pf->GetLmaAddress ());
  if (pf->GetHomeNetworkPrefixes ().size () > 0)
    {
      bule->SetHomeNetworkPrefixes (pf->GetHomeNetworkPrefixes ());
    }

  //XXX: how to determine proper HI(Handoff Indicator) value??
  bule->SetHandoffIndicator (Ipv6MobilityHeader::OPT_NEW_ATTACHMENT); //OPT_HI_HANDOFF_STATE_UNKNOWN

  Ipv6Address lla = GetLinkLocalAddress (bule->GetLmaAddress ());
  if (!lla.IsAny ())
    {
      bule->SetMagLinkAddress (lla);
    }

  bule->SetIfIndex (m_ifIndex);

  // Set LTE parameters
  bule->SetImsi (imsi);
  bule->SetTunnelId (teid);

  // Preset header information
  bule->SetLastBindingUpdateSequence (GetSequence ());
  // Cut to micro-seconds
  bule->SetLastBindingUpdateTime (MicroSeconds (Simulator::Now ().GetMicroSeconds ()));

  Ptr<Packet> p = BuildPbu (bule);
  //save packet
  bule->SetPbuPacket (p);
  // Reset (for the first registration)
  bule->ResetRetryCount ();

  // send PBU
  SendMessage (p->Copy (), bule->GetLmaAddress (), 64);

  bule->StartRetransTimer ();
  if (bule->IsReachable ())
    {
      bule->MarkRefreshing ();
    }
  else
    {
      bule->MarkUpdating ();
    }
}

uint8_t Pmipv6Mag::HandlePba (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  Ptr<Packet> p = packet->Copy ();

  Ipv6MobilityBindingAckHeader pba;
  Ipv6MobilityOptionBundle bundle;

  p->RemoveHeader (pba);

  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = GetNode ()->GetObject<Ipv6MobilityDemux> ();
  NS_ASSERT (ipv6MobilityDemux);
  Ptr<Ipv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility (pba.GetMhType ());
  NS_ASSERT (ipv6Mobility);

  uint8_t length = ((pba.GetHeaderLen () + 1) << 3) - pba.GetOptionsOffset ();
  ipv6Mobility->ProcessOptions (packet, pba.GetOptionsOffset (), length, bundle);
  //option check
  // Error Process for Mandatory Options
  if (bundle.GetMnIdentifier ().IsEmpty () ||
      bundle.GetHomeNetworkPrefixes ().size () == 0 ||
      bundle.GetAccessTechnologyType () == Ipv6MobilityHeader::OPT_ATT_RESERVED ||
      bundle.GetHandoffIndicator () == Ipv6MobilityHeader::OPT_HI_RESERVED ||
      bundle.GetTimestamp ().GetMicroSeconds () == 0)
    {
      NS_LOG_LOGIC ("PBA Option missing.. Ignored.");
      return 0;
    }
  // Check timestamp must be less than current time.
  if (bundle.GetTimestamp () > Simulator::Now ())
    {
      NS_LOG_LOGIC ("Timestamp is mismatched. Ignored.");
      return 0;
    }

  BindingUpdateList::Entry *bule = m_buList->Lookup (bundle.GetMnIdentifier ());
  if (bule == 0)
    {
      NS_LOG_LOGIC ("No matched PBA for PBU. Ignored.");
      return 0;
    }

  // check whether Timestamp and Sequence of PBA matches with those sent in PBU (or the entry in BUL).
  if (bule->GetLastBindingUpdateSequence () != pba.GetSequence () ||
      bule->GetLastBindingUpdateTime () != bundle.GetTimestamp ())
    {
      NS_LOG_LOGIC ("Sequence or Timestamp mismatch. Ignored. this: "
                    << bule->GetLastBindingUpdateTime ()
                    << ", from: "
                    << bundle.GetTimestamp ());
      return 0;
    }

  // Check status code
  switch (pba.GetStatus ())
    {
    case Ipv6MobilityHeader::BA_STATUS_BINDING_UPDATE_ACCEPTED:
    {
      bule->StopRetransTimer ();
      bule->SetPbuPacket (0);

      // Update information
      bule->SetLmaAddress (src);
      bule->SetHomeNetworkPrefixes (bundle.GetHomeNetworkPrefixes ());
      bule->SetReachableTime (Seconds (pba.GetLifetime ()));

      if (pba.GetLifetime () > 0)
        {
          if (bule->IsUpdating ())
            {
              if (IsLteMag ())
                {
                  m_setIp (bule->GetImsi (), bule->GetHomeNetworkPrefixes().front ());
                  SetupLteRadvdInterface (bule);
                }
              else
                {
                  // Register radvd interface
                  SetupRegularRadvdInterface (bule);
                }
              // Create tunnel & setup routing
              SetupTunnelAndRouting (bule);
            }
          bule->MarkReachable ();

          // Setup lifetime
          bule->StopRefreshTimer ();
          bule->StartRefreshTimer ();
          bule->StopReachableTimer ();
          bule->StartReachableTimer ();
        }
      else
        {
          // Unregistering
          ClearRadvdInterface (bule);
          ClearTunnelAndRouting (bule);
          m_buList->Remove (bule);
        }
      break;
    }
    default:
      NS_LOG_LOGIC ("Error occurred code=" << pba.GetStatus ());
      break;
    }

  return 0;
}
uint8_t Pmipv6Mag::HandleHur (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  Ptr<Packet> p = packet->Copy ();
  BindingUpdateList::Entry *bule_new;
  Ipv6MobilityBindingAckHeader hur;
  Ipv6MobilityOptionBundle bundle;

  p->RemoveHeader (hur);

  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = GetNode ()->GetObject<Ipv6MobilityDemux> ();
  NS_ASSERT (ipv6MobilityDemux);
  Ptr<Ipv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility (hur.GetMhType ());
  NS_ASSERT (ipv6Mobility);

  uint8_t length = ((hur.GetHeaderLen () + 1) << 3) - hur.GetOptionsOffset ();
  ipv6Mobility->ProcessOptions (packet, hur.GetOptionsOffset (), length, bundle);
  //option check
  // Error Process for Mandatory Options
  if (bundle.GetMnIdentifier ().IsEmpty () || bundle.GetHomeNetworkPrefixes ().size () == 0 || bundle.GetAccessTechnologyType () == Ipv6MobilityHeader::OPT_ATT_RESERVED || bundle.GetTimestamp ().GetMicroSeconds () == 0)						//bundle.GetHandoffIndicator () == Ipv6MobilityHeader::OPT_HI_RESERVED ||
    {
      NS_LOG_LOGIC ("HUR Option missing.. Ignored.");
      return 0;
    }
  // Check timestamp must be less than current time.
  if (bundle.GetTimestamp () > Simulator::Now ())
    {
      NS_LOG_LOGIC ("Timestamp is mismatched. Ignored.");
      return 0;
    }
  BindingUpdateList::Entry *bule = m_buList->Lookup (bundle.GetMnIdentifier ());
  if (bule == 0)
    {
      NS_LOG_LOGIC ("No matched HUR for HUR. Ignored.");
      return 0;
    }
  std::list<Ipv6Address> new_hnps;
  switch (hur.GetStatus ())
    {
    case Ipv6MobilityHeader::BA_STATUS_BINDING_UPDATE_ACCEPTED:
    {
	  NS_LOG_LOGIC("Adding Entry to buList");
		  bule_new=m_buList->Lookup(bundle.GetMnIdentifier ());
		  std::list<Ipv6Address> prev_hnps=bule_new->GetHomeNetworkPrefixes();
		  new_hnps=bundle.GetHomeNetworkPrefixes();
		  for (std::list<Ipv6Address>::iterator iterator = new_hnps.begin(); iterator != new_hnps.end(); ++iterator)
			  prev_hnps.push_back(*iterator);
		  ClearTunnelAndRouting(bule_new);
		  bule_new->SetHomeNetworkPrefixes (prev_hnps);
		  SetupLteRadvdInterface (bule_new);

		  SetupTunnelAndRouting (bule_new);
      break;
    }
    default:
      NS_LOG_LOGIC ("Error occurred code=" << hur.GetStatus ());
      break;
    }
  Ptr<Packet> pktHua=BuildHua(bule_new,new_hnps);
  SendMessage (pktHua, src, 64);
  return 0;
}

bool Pmipv6Mag::SetupTunnelAndRouting (BindingUpdateList::Entry *bule)
{
  NS_LOG_FUNCTION (this << bule);

  // Create tunnel
  Ptr<Ipv6TunnelL4Protocol> th = GetNode ()->GetObject<Ipv6TunnelL4Protocol> ();
  NS_ASSERT (th);

  uint16_t tunnelIf = th->AddTunnel (bule->GetLmaAddress ());

  bule->SetTunnelIfIndex (tunnelIf);

  //routing setup by static routing protocol
  Ipv6StaticRoutingHelper staticRoutingHelper;
  Ipv6StaticSourceRoutingHelper sourceRoutingHelper;

  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  Ptr<Ipv6StaticRouting> staticRouting = staticRoutingHelper.GetStaticRouting (ipv6);
  Ptr<Ipv6StaticSourceRouting> sourceRouting = sourceRoutingHelper.GetStaticSourceRouting (ipv6);
  NS_ASSERT (staticRouting && sourceRouting);

  std::list<Ipv6Address> hnpList = bule->GetHomeNetworkPrefixes ();
  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
    {
      NS_LOG_LOGIC ("Add Route to " << (*i) << "/64 via " << (uint32_t) bule->GetIfIndex ());
      staticRouting->AddNetworkRouteTo ((*i), Ipv6Prefix (64), bule->GetIfIndex ());

      NS_LOG_LOGIC ("Add Source Route from " << (*i) << "/64 via " << (uint32_t) bule->GetTunnelIfIndex ());
      sourceRouting->AddNetworkRouteFrom ((*i), Ipv6Prefix (64), bule->GetTunnelIfIndex ());
    }

  return true;
}

void Pmipv6Mag::ClearTunnelAndRouting (BindingUpdateList::Entry *bule)
{
  NS_LOG_FUNCTION (this << bule);

  //routing setup by static routing protocol
  Ipv6StaticRoutingHelper staticRoutingHelper;
  Ipv6StaticSourceRoutingHelper sourceRoutingHelper;

  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();

  Ptr<Ipv6StaticRouting> staticRouting = staticRoutingHelper.GetStaticRouting (ipv6);
  Ptr<Ipv6StaticSourceRouting> sourceRouting = sourceRoutingHelper.GetStaticSourceRouting (ipv6);

  NS_ASSERT (staticRouting && sourceRouting);

  std::list<Ipv6Address> hnpList = bule->GetHomeNetworkPrefixes ();

  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
    {
      NS_LOG_LOGIC ("Remove Route to " << (*i) << "/64 via " << (uint32_t)bule->GetIfIndex ());
      staticRouting->RemoveRoute ((*i), Ipv6Prefix (64), bule->GetIfIndex (), (*i));

      NS_LOG_LOGIC ("Remove Source Route from " << (*i) << "/64 via " << (uint32_t)bule->GetTunnelIfIndex ());
      sourceRouting->RemoveRoute ((*i), Ipv6Prefix (64), bule->GetTunnelIfIndex (), (*i));
    }

  //remove tunnel
  Ptr<Ipv6TunnelL4Protocol> th = GetNode ()->GetObject<Ipv6TunnelL4Protocol> ();
  NS_ASSERT (th);

  th->RemoveTunnel (bule->GetLmaAddress ());
  bule->SetTunnelIfIndex (-1);
}

void Pmipv6Mag::SetupRegularRadvdInterface (BindingUpdateList::Entry *bule)
{
  NS_LOG_FUNCTION (this << bule);

  uint32_t ifIndex = bule->GetIfIndex ();
  Ptr<UnicastRadvdInterface> uri = Create<UnicastRadvdInterface> (ifIndex, 5000, 1000);
  Identifier linkId = bule->GetMnLinkIdentifier ();

  Mac48Address phyId;
  uint8_t buf[Identifier::MAX_SIZE];
  linkId.CopyTo (buf, Identifier::MAX_SIZE);
  phyId.CopyFrom (buf);
  uri->SetPhysicalAddress (phyId);

  std::list<Ipv6Address> hnpList = bule->GetHomeNetworkPrefixes ();
  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
    {
      Ptr<RadvdPrefix> prefix = Create<RadvdPrefix> ((*i), 64, 3, 5);
      uri->AddPrefix (prefix);
    }

  GetRadvd ()->AddConfiguration (uri);
  bule->SetRadvdIfIndex (uri->GetInterface ());
}

void Pmipv6Mag::SetupLteRadvdInterface (BindingUpdateList::Entry *bule)
{
  NS_LOG_FUNCTION (this << bule);

  Ptr<UnicastRadvdInterface> uri = Create<UnicastRadvdInterface> (bule->GetIfIndex (), 5000, 1000, UnicastRadvdInterface::LTE);
  uri->SetImsi (bule->GetImsi ());
  uri->SetTunnelId (bule->GetTunnelId ());
  std::list<Ipv6Address> hnpList = bule->GetHomeNetworkPrefixes ();
  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
    {
      Ptr<RadvdPrefix> prefix = Create<RadvdPrefix> ((*i), 64, 3, 5);
      uri->AddPrefix (prefix);
    }
  GetRadvd ()->AddConfiguration (uri);
  bule->SetRadvdIfIndex (uri->GetInterface ());
}

void Pmipv6Mag::ClearRadvdInterface (BindingUpdateList::Entry *bule)
{
  NS_LOG_FUNCTION (this << bule);

  GetRadvd ()->RemoveConfiguration (bule->GetRadvdIfIndex ());
  bule->SetRadvdIfIndex (-1);
}

} /* namespace ns3 */

