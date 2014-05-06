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
#include "ns3/boolean.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"

//#include "ns3/wifi-net-device.h"
//#include "ns3/wifi-mac.h"
//#include "ns3/regular-wifi-mac.h"
//#include "ns3/wimax-net-device.h"

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
#include "pmipv6-profile.h"
#include "pmipv6-prefix-pool.h"

#include "pmipv6-lma.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("Pmipv6Lma");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Pmipv6Lma);

Pmipv6Lma::Pmipv6Lma ()
 : m_bCache (0),
   m_prefixPool (0)
{
}

Pmipv6Lma::~Pmipv6Lma ()
{
  m_bCache = 0;
  m_prefixPool = 0;
}

void Pmipv6Lma::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_bCache->Flush ();
  m_bCache = 0;
  m_prefixPool = 0;
  Pmipv6Agent::DoDispose ();
}

Ptr<Pmipv6PrefixPool> Pmipv6Lma::GetPrefixPool () const
{
  NS_LOG_FUNCTION_NOARGS ();
  
  return m_prefixPool;
}

void Pmipv6Lma::SetPrefixPool (Ptr<Pmipv6PrefixPool> pool)
{
  NS_LOG_FUNCTION (this << pool);
  
  m_prefixPool = pool;
}

void Pmipv6Lma::NotifyNewAggregate ()
{
  if (GetNode () == 0)
    {
      Ptr<Node> node = this->GetObject<Node> ();
      m_bCache = CreateObject<BindingCache> ();
      SetNode (node);
      m_bCache->SetNode (node);
    }
  Pmipv6Agent::NotifyNewAggregate ();
}

Ptr<Packet> Pmipv6Lma::BuildPba (BindingCache::Entry *bce, uint8_t status)
{
  NS_LOG_FUNCTION (this << bce << status);
  
  Ptr<Packet> p = Create<Packet> ();
  
  Ipv6MobilityBindingAckHeader pba;
  
  Ipv6MobilityOptionMobileNodeIdentifierHeader nai;
  Ipv6MobilityOptionHomeNetworkPrefixHeader hnph;
  Ipv6MobilityOptionHandoffIndicatorHeader hih;
  Ipv6MobilityOptionAccessTechnologyTypeHeader atth;
  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader mnllidh;
  Ipv6MobilityOptionTimestampHeader timestamph;

  pba.SetStatus (status);
  pba.SetFlagP (true);
  pba.SetSequence (bce->GetLastBindingUpdateSequence ());
  pba.SetLifetime ((uint16_t)bce->GetReachableTime ().GetSeconds ());
  
  nai.SetSubtype (1);
  nai.SetNodeIdentifier (bce->GetMnIdentifier ());
  pba.AddOption (nai);
  
  std::list<Ipv6Address> hnpList = bce->GetHomeNetworkPrefixes ();
  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end(); i++)
    {
      hnph.SetPrefix ((*i));
      hnph.SetPrefixLength (64);
      pba.AddOption (hnph);
    }
  
  hih.SetHandoffIndicator (bce->GetHandoffIndicator ());
  pba.AddOption (hih);
  
  atth.SetAccessTechnologyType (bce->GetAccessTechnologyType ());
  pba.AddOption (atth);
  
  mnllidh.SetLinkLayerIdentifier (bce->GetMnLinkIdentifier ());
  pba.AddOption (mnllidh);
  
  if (!bce->GetMagLinkAddress ().IsAny ())
    {
      Ipv6MobilityOptionLinkLocalAddressHeader llah;
      llah.SetLinkLocalAddress (bce->GetMagLinkAddress ());
      pba.AddOption (llah);
    }
  
  timestamph.SetTimestamp (bce->GetLastBindingUpdateTime ());
  pba.AddOption (timestamph);
  
  p->AddHeader (pba);
  return p;
}

Ptr<Packet> Pmipv6Lma::BuildPba(Ipv6MobilityBindingUpdateHeader pbu, Ipv6MobilityOptionBundle bundle, uint8_t status)
{
  NS_LOG_FUNCTION (this << status);
  
  Ptr<Packet> p = Create<Packet> ();
  
  Ipv6MobilityBindingAckHeader pba;
  
  Ipv6MobilityOptionMobileNodeIdentifierHeader nai;
  Ipv6MobilityOptionHomeNetworkPrefixHeader hnph;
  Ipv6MobilityOptionHandoffIndicatorHeader hih;
  Ipv6MobilityOptionAccessTechnologyTypeHeader atth;
  Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader mnllidh;
  Ipv6MobilityOptionTimestampHeader timestamph;

  pba.SetSequence (pbu.GetSequence ());
  pba.SetFlagP (true);
  pba.SetStatus (status);
  
  nai.SetSubtype (1);
  nai.SetNodeIdentifier (bundle.GetMnIdentifier ());
  pba.AddOption (nai);
  
  std::list<Ipv6Address> hnpList = bundle.GetHomeNetworkPrefixes ();
  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
    {
      hnph.SetPrefix ((*i));
      hnph.SetPrefixLength (64);
      pba.AddOption (hnph);
    }
  
  hih.SetHandoffIndicator (bundle.GetHandoffIndicator ());
  pba.AddOption (hih);
  
  atth.SetAccessTechnologyType (bundle.GetAccessTechnologyType ());
  pba.AddOption (atth);
  
  mnllidh.SetLinkLayerIdentifier (bundle.GetMnLinkIdentifier ());
  pba.AddOption (mnllidh);
  
  if (!bundle.GetMagLinkAddress ().IsAny ())
    {
      Ipv6MobilityOptionLinkLocalAddressHeader llah;
      
      llah.SetLinkLocalAddress (bundle.GetMagLinkAddress ());
      pba.AddOption (llah);
    }
  
  timestamph.SetTimestamp (bundle.GetTimestamp ());
  pba.AddOption (timestamph);
  
  p->AddHeader (pba);
  
  return p;
}

Ptr<Packet> Pmipv6Lma::BuildHur (BindingCache::Entry *bce_new,BindingCache::Entry *bce_old, uint8_t status)
{
NS_LOG_FUNCTION (this << bce_new<<"  "<<bce_old << status);

 Ptr<Packet> p = Create<Packet> ();

 Ipv6MobilityBindingAckHeader hur;

 Ipv6MobilityOptionMobileNodeIdentifierHeader MnIdh;
 Ipv6MobilityOptionHomeNetworkPrefixHeader hnph;
 Ipv6MobilityOptionAccessTechnologyTypeHeader atth;
 Ipv6MobilityOptionMobileNodeLinkLayerIdentifierHeader mnllidh;
 Ipv6MobilityOptionTimestampHeader timestamph;

 hur.SetStatus (status);
 hur.SetFlagP (true);
 hur.SetFlagK(true);
 hur.SetFlagT(true);
 //hur.SetFlagA(true);
 hur.SetSequence (bce_old->GetLastBindingUpdateSequence ());
 hur.SetLifetime ((uint16_t)bce_old->GetReachableTime ().GetSeconds ());

 MnIdh.SetSubtype (1);
 MnIdh.SetNodeIdentifier (bce_old->GetMnIdentifier ());
 hur.AddOption (MnIdh);

 //HOW MANY HNP'S?
 //std::list<Ipv6Address> hnpList = bce->GetHomeNetworkPrefixes();
 /*std::list<Ipv6Address> hnpList;
 hnpList.push_back(bce->GetHomeNetworkPrefixes().back()); //sending last added hnp
 for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end(); i++)
   {
     hnph.SetPrefix ((*i));
     hnph.SetPrefixLength (64);
     hur.AddOption (hnph);
   }*/
 list <Ipv6Address>::iterator iter = bce_new->GetHomeNetworkPrefixes().end();
 hnph.SetPrefix (*(--iter));
 hnph.SetPrefixLength (64);
 hur.AddOption (hnph);


 atth.SetAccessTechnologyType (bce_old->GetAccessTechnologyType ());
 hur.AddOption (atth);

 mnllidh.SetLinkLayerIdentifier (bce_old->GetMnLinkIdentifier ());
 hur.AddOption (mnllidh);

 timestamph.SetTimestamp (bce_old->GetLastBindingUpdateTime ());
 hur.AddOption (timestamph);

 p->AddHeader (hur);
 return p;
}

uint8_t Pmipv6Lma::HandlePbu(Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  
  Ptr<Packet> p = packet->Copy ();
  
  Ipv6MobilityBindingUpdateHeader pbu;
  Ipv6MobilityOptionBundle bundle;
  
  p->RemoveHeader (pbu);
  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = GetNode ()->GetObject<Ipv6MobilityDemux> ();
  NS_ASSERT (ipv6MobilityDemux);
  Ptr<Ipv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility (pbu.GetMhType ());
  NS_ASSERT (ipv6Mobility);
  
  uint8_t length = ((pbu.GetHeaderLen () + 1) << 3) - pbu.GetOptionsOffset ();
  ipv6Mobility->ProcessOptions (packet, pbu.GetOptionsOffset (), length, bundle);
  
  uint8_t errStatus = 0;
  BindingCache::Entry *bce = 0;
  BindingCache::Entry *bce_new = 0;
  Pmipv6Profile::Entry *pf = 0;
  bool Another_Interface_Att = false;
  bool delayedRegister = false;
  
  /* RFC5213 5.3.1 - 4 Check MN Identifier*/
  if (bundle.GetMnIdentifier ().IsEmpty ())
    {
      errStatus = Ipv6MobilityHeader::BA_STATUS_MISSING_MN_IDENTIFIER_OPTION;
    }
  /* 5.3.1 - 7 Check requested MN belongs to this LMA */
  else if ((pf = GetProfile ()->LookupMnId (bundle.GetMnIdentifier ())) == 0)
    {
      errStatus = Ipv6MobilityHeader::BA_STATUS_NOT_LMA_FOR_THIS_MOBILE_NODE;
    }
  /* 5.3.1 - 9 Check Timestamp and sequence */
  else if (bundle.GetTimestamp ().GetMicroSeconds () != 0 && bundle.GetTimestamp ()>Simulator::Now ())
    {
      errStatus = Ipv6MobilityHeader::BA_STATUS_TIMESTAMP_MISMATCH;
    }  
  /* 5.3.1 - 10 Check whether Home Network Prefix(es) exist(s) */    
  else if (bundle.GetHomeNetworkPrefixes ().size () == 0)
    {
      errStatus = Ipv6MobilityHeader::BA_STATUS_MISSING_HOME_NETWORK_PREFIX_OPTION;
    }
  /* 5.3.1 - 11 Check Handoff Indicator presents */
  else if (bundle.GetHandoffIndicator () == Ipv6MobilityHeader::OPT_HI_RESERVED)
    {
      errStatus = Ipv6MobilityHeader::BA_STATUS_MISSING_HANDOFF_INDICATOR_OPTION;
    }
  /* 5.3.1 - 12 Check Access Technology Type presents */
  else if (bundle.GetAccessTechnologyType () == Ipv6MobilityHeader::OPT_ATT_RESERVED)
    {
      errStatus = Ipv6MobilityHeader::BA_STATUS_MISSING_ACCESS_TECH_TYPE_OPTION;
    }
  /* 5.3.1 - 13 Binding Cache entry existence test */
  else
    {
      Identifier mnId = bundle.GetMnIdentifier ();
      Identifier mnLinkId = bundle.GetMnLinkIdentifier ();
      std::list<Ipv6Address> hnpList;
      
      // copy home network prefixes which has NON_ZERO prefix
      if (bundle.GetHomeNetworkPrefixes ().size () > 0)
        {
          std::list<Ipv6Address> hnpTemp = bundle.GetHomeNetworkPrefixes ();
          for (std::list<Ipv6Address>::iterator i = hnpTemp.begin (); i != hnpTemp.end (); i++)
            {
              if (!(*i).IsAny ())
                {
                  hnpList.push_back((*i));
                }
            }
        }
      
      if (hnpList.size () > 0)
        {
          bool allMatched = false;
          /* 5.4.1.1 any NON_ZERO Home Network Prefix presents */
          bce = m_bCache->Lookup (mnId, hnpList, allMatched);
          
          /* 5.4.1.1 - 2 Creating a new Mobility session */
          if (bce == 0)
            {
              
            }
          else
            {
              /* 5.4.1.1 - 4 not all prefix set matched */
              if (!allMatched)
                {
                  bce = 0;
                  errStatus = Ipv6MobilityHeader::BA_STATUS_BCE_PBU_PREFIX_SET_DO_NOT_MATCH;
                }
              else
                {
                  NS_LOG_LOGIC ("BCE with all-matched Home Network Prefixes found.");
                  /* 5.4.1.1 - 5 Updating BCE */
                }
            }
        }
      else if (!mnLinkId.IsEmpty ())
        {
          // 5.4.1.2
          bce = m_bCache->Lookup (mnId, bundle.GetAccessTechnologyType (), mnLinkId);
          
          // 5.4.1.2 - 3, 4
          if (bce == 0)
            {
              // 5.4.1.2 - 3
              if (bundle.GetHandoffIndicator () == Ipv6MobilityHeader::OPT_HI_HANDOFF_BETWEEN_DIFFERENT_INTERFACES)
                {
                  bce = m_bCache->Lookup (mnId);
                }
              // 5.4.1.2 - 4
              else if (bundle.GetHandoffIndicator () == Ipv6MobilityHeader::OPT_HI_HANDOFF_STATE_UNKNOWN)
                {
                  bce = m_bCache->Lookup (mnId);
                  delayedRegister = true;
                }
            }
        }
      else
        {
          //5.4.1.3
          bce = m_bCache->Lookup (mnId);
          delayedRegister = true;
        }
        
      // BCE exists
      if (bce != 0)
        {
          if (pbu.GetLifetime () > 0)
            {
              if ((bce->GetProxyCoa () == src) || bce->IsDeregistering ())
                {
                  // update BCE
                  bce->SetProxyCoa (src);
                  bce->SetMnLinkIdentifier (mnLinkId);
                  bce->SetAccessTechnologyType (bundle.GetAccessTechnologyType ());
                  bce->SetHandoffIndicator (bundle.GetHandoffIndicator ());
                  bce->SetMagLinkAddress (bundle.GetMagLinkAddress ());
                  bce->SetLastBindingUpdateTime (bundle.GetTimestamp ());
                  bce->SetReachableTime (Seconds (pbu.GetLifetime ()));
                  bce->SetLastBindingUpdateSequence (pbu.GetSequence ());
                  
                  bce->MarkReachable ();
                  
                  //start lifetime timer
                  bce->StopReachableTimer ();
                  bce->StartReachableTimer ();
                }
              else if (bce->GetProxyCoa () != src)
                {
                  NS_LOG_LOGIC ("PBU from another MAG.");
                  if (bundle.GetHandoffIndicator () == Ipv6MobilityHeader::OPT_HI_HANDOFF_STATE_UNKNOWN && delayedRegister == true)
                    {
                      NS_LOG_LOGIC ("Cannot determine handoff state, performing delayed-registration");
                      if (!bce->IsRegistering ())
                        {
                          BindingCache::Entry *bce_temp = bce->Copy ();
                          
                          bce_temp->SetProxyCoa (src);
                          
                          bce_temp->SetMnLinkIdentifier (mnLinkId);
                          bce_temp->SetAccessTechnologyType (bundle.GetAccessTechnologyType ());
                          bce_temp->SetHandoffIndicator (bundle.GetHandoffIndicator ());
                          bce_temp->SetMagLinkAddress (bundle.GetMagLinkAddress ());
                          
                          bce_temp->SetLastBindingUpdateTime (bundle.GetTimestamp ());
                          bce_temp->SetReachableTime (Seconds (pbu.GetLifetime ()));
                          bce_temp->SetLastBindingUpdateSequence (pbu.GetSequence ());
                          
                          bce_temp->SetHomeNetworkPrefixes (bundle.GetHomeNetworkPrefixes ());
                          
                          bce->SetTentativeEntry (bce_temp);
                          
                          bce->MarkRegistering ();
                      
                          bce->StopRegisterTimer ();
                          bce->StartRegisterTimer ();
                        }
                      else
                        {
                          NS_LOG_LOGIC ("Already in delayed registration. Skipped.");
                        }
                      return 0;
                    }
                  else if (bce->GetAccessTechnologyType()==bundle.GetAccessTechnologyType ())
                    {
                      NS_LOG_LOGIC ("Handoff in two different MAGs for the same interface");
                      
                      //update BCE
                      bce->SetProxyCoa (src);
                      
                      bce->SetMnLinkIdentifier (mnLinkId);
                      bce->SetAccessTechnologyType (bundle.GetAccessTechnologyType ());
                      bce->SetHandoffIndicator (bundle.GetHandoffIndicator ());
                      bce->SetMagLinkAddress (bundle.GetMagLinkAddress ());
                      
                      bce->SetLastBindingUpdateTime (bundle.GetTimestamp ());
                      bce->SetReachableTime (Seconds (pbu.GetLifetime ()));
                      bce->SetLastBindingUpdateSequence (pbu.GetSequence ());
                      
                      ModifyTunnelAndRouting (bce);

                      bce->MarkReachable ();
                      
                      //start lifetime timer
                      bce->StopReachableTimer ();
                      bce->StartReachableTimer ();
                    }
                  else if(bce->GetAccessTechnologyType()!=bundle.GetAccessTechnologyType ())
                  {
                	  NS_LOG_LOGIC ("Another interface of same node connected to Another MAG");
                	  Another_Interface_Att=true;

                      NS_LOG_LOGIC ("Creating new Binding Cache Entry");

                      //bce_new = m_bCache->Add (mnId);
                      bce_new=m_bCache->Add(bce->GetMnIdentifier());
                      bce_new->SetProxyCoa (src);
                      bce_new->SetMnLinkIdentifier (mnLinkId);
                      bce_new->SetAccessTechnologyType (bundle.GetAccessTechnologyType ());
                      bce_new->SetHandoffIndicator (bundle.GetHandoffIndicator ());
                      bce_new->SetMagLinkAddress (bundle.GetMagLinkAddress ());
                      bce_new->SetLastBindingUpdateTime (bundle.GetTimestamp ());
                      bce_new->SetReachableTime (Seconds (pbu.GetLifetime ()));
                      bce_new->SetLastBindingUpdateSequence (pbu.GetSequence ());
                      bce_new->SetNext(bce);

                      std::list<Ipv6Address> hnpList=bce->GetHomeNetworkPrefixes();
					  Ipv6Address prefix = m_prefixPool->Assign ();
					  NS_LOG_LOGIC ("Assign new Prefix for another interface from Pool: " << prefix);
					  hnpList.push_back (prefix);
					  bce_new->SetHomeNetworkPrefixes (hnpList);
					  if (pf)
					  {
						pf->SetHomeNetworkPrefixes (hnpList);
					  }
					  SetupTunnelAndRouting (bce_new);
					  bce_new->MarkReachable ();
					  //bce->SetNext(bce_new);
					  // start lifetime timer
					  bce_new->StopReachableTimer ();
					  bce_new->StartReachableTimer ();
                      BindingCache::Entry *bce_iterator2=m_bCache->Lookup(bce->GetMnIdentifier());
					  while(bce_iterator2!=0)
					  {
						  NS_LOG_LOGIC (bce_iterator2->GetAccessTechnologyType()<<" "<<bce_iterator2->GetNext());
						  bce_iterator2=bce_iterator2->GetNext();
					  }

                  }
                } // PBU from another MAG.
            } // PBU lifetime > 0.
          else
            {
              NS_LOG_LOGIC ("Lifetime is zero, Unregistering.");
              if (bce->GetProxyCoa () == src)
                {
                  // Unregistering
                  bce->MarkDeregistering ();
                  bce->StartDeregisterTimer ();
                }
            }
        } // BCE exists.
      else
        {
          if (pbu.GetLifetime () > 0)
            {
              // No Binding Cache Exists
              NS_LOG_LOGIC ("Creating new Binding Cache Entry");
              
              bce = m_bCache->Add (mnId);

              bce->SetProxyCoa (src);
              
              bce->SetMnLinkIdentifier (mnLinkId);
              bce->SetAccessTechnologyType (bundle.GetAccessTechnologyType ());
              bce->SetHandoffIndicator (bundle.GetHandoffIndicator ());
              bce->SetMagLinkAddress (bundle.GetMagLinkAddress ());
              
              bce->SetLastBindingUpdateTime (bundle.GetTimestamp ());
              bce->SetReachableTime (Seconds (pbu.GetLifetime ()));
              bce->SetLastBindingUpdateSequence (pbu.GetSequence ());
              bce->SetNext(0);
              // Allocate new prefix
              std::list<Ipv6Address> hnpList;
              // Assign HNP from profile if present, otherwise assign from Prefix Pool.
              if (pf && pf->GetHomeNetworkPrefixes ().size () > 0)
                {
                  bce->SetHomeNetworkPrefixes (pf->GetHomeNetworkPrefixes ());
                }
              else
                {
                  Ipv6Address prefix = m_prefixPool->Assign ();
                  NS_LOG_LOGIC ("Assign new Prefix from Pool: " << prefix);
                  hnpList.push_back (prefix);
                  bce->SetHomeNetworkPrefixes (hnpList);
                  // Assign the HNPs to Profile as well.
                  if (pf)
                    {
                      pf->SetHomeNetworkPrefixes (hnpList);
                    }
                }
                
              // create tunnel
              SetupTunnelAndRouting (bce);
              
              bce->MarkReachable ();
              // start lifetime timer
              bce->StartReachableTimer ();
            } // No BCE exists and PBU lifetime > 0.
        }
      errStatus = Ipv6MobilityHeader::BA_STATUS_BINDING_UPDATE_ACCEPTED;
    }
    
  // Send PBA
  Ptr<Packet> pktPba;
  if (bce != 0 && !Another_Interface_Att)
    {
      pktPba = BuildPba (bce, errStatus);
    }
  else if(bce != 0 && Another_Interface_Att)
  {
	  //send pba with two hnp
	  pktPba = BuildPba (bce_new, errStatus);

	  //for all previous mags send hur

	  BindingCache::Entry *bce_iterator=bce_new->GetNext();
	  NS_LOG_LOGIC("Sending HURs to all MAGs");
	  while(bce_iterator !=NULL && bce_iterator !=0 && !(bce_iterator->IsEqual(bce_new)))
	  {
		  Ptr<Packet> pktHur=BuildHur(bce_new, bce_iterator,errStatus);
		  NS_LOG_LOGIC("Hur for  "<< bce_iterator->GetProxyCoa());
		  SendMessage (pktHur, bce_iterator->GetProxyCoa(), 64);
		  bce_iterator=bce_iterator->GetNext();
	  }
	  SendMessage (pktPba, src, 64);
	  return 0;
  }
  else
    {
      pktPba = BuildPba (pbu, bundle, errStatus);
    }
  SendMessage (pktPba, src, 64);
  return 0;
}
uint8_t Pmipv6Lma::HandleHua(Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface){
	NS_LOG_FUNCTION (this << packet << src << dst << interface);
	  Ptr<Packet> p = packet->Copy ();
	  Ipv6MobilityBindingUpdateHeader hua;
	  Ipv6MobilityOptionBundle bundle;

	  p->RemoveHeader (hua);

	  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = GetNode ()->GetObject<Ipv6MobilityDemux> ();
	  NS_ASSERT (ipv6MobilityDemux);
	  Ptr<Ipv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility (hua.GetMhType ());
	  NS_ASSERT (ipv6Mobility);

	  uint8_t length = ((hua.GetHeaderLen () + 1) << 3) - hua.GetOptionsOffset ();
	  ipv6Mobility->ProcessOptions (packet, hua.GetOptionsOffset (), length, bundle);
	  //option check
	  // Error Process for Mandatory Options
	  if (bundle.GetMnIdentifier ().IsEmpty () || bundle.GetHomeNetworkPrefixes ().size () == 0 || bundle.GetAccessTechnologyType () == Ipv6MobilityHeader::OPT_ATT_RESERVED || bundle.GetTimestamp ().GetMicroSeconds () == 0)						//bundle.GetHandoffIndicator () == Ipv6MobilityHeader::OPT_HI_RESERVED ||
	    {
	      NS_LOG_LOGIC ("HUA Option missing.. Ignored.");
	      return 0;
	    }
	  // Check timestamp must be less than current time.
	  if (bundle.GetTimestamp () > Simulator::Now ())
	    {
	      NS_LOG_LOGIC ("Timestamp is mismatched. Ignored.");
	      return 0;
	    }
	  BindingCache::Entry *bule = m_bCache->Lookup (bundle.GetMnIdentifier (),bundle.GetAccessTechnologyType(),bundle.GetMnLinkIdentifier());
	  if (bule == 0)
	    {
	      NS_LOG_LOGIC ("No matched HUA for HUA. Ignored.");
	      return 0;
	    }
	  //ClearTunnelAndRouting(bule_new);
	  std::list<Ipv6Address> prev_hnps=bule->GetHomeNetworkPrefixes();
	  std::list<Ipv6Address> new_hnps=bundle.GetHomeNetworkPrefixes();
	  		  for (std::list<Ipv6Address>::iterator iterator = new_hnps.begin(); iterator != new_hnps.end(); ++iterator)
	  			  prev_hnps.push_back(*iterator);
	  bule->SetHomeNetworkPrefixes (prev_hnps);
	  //SetupLteRadvdInterface (bule);
	  SetupTunnelAndRouting (bule);
	return 0;
}
bool Pmipv6Lma::SetupTunnelAndRouting (BindingCache::Entry *bce)
{
  NS_LOG_FUNCTION (this << bce);
  
  // Create tunnel
  Ptr<Ipv6TunnelL4Protocol> th = GetNode ()->GetObject<Ipv6TunnelL4Protocol> ();
  NS_ASSERT (th);

  uint16_t tunnelIf = th->AddTunnel (bce->GetProxyCoa ());
  NS_LOG_INFO("Tunnel Added  " << bce->GetProxyCoa () <<"  index  "<<tunnelIf);
  bce->SetTunnelIfIndex (tunnelIf);
  
  // Routing setup by static routing protocol
  Ipv6StaticRoutingHelper staticRoutingHelper;
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  Ptr<Ipv6StaticRouting> staticRouting = staticRoutingHelper.GetStaticRouting (ipv6);
  
  std::list<Ipv6Address> hnpList = bce->GetHomeNetworkPrefixes ();
  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
    {
      NS_LOG_LOGIC ("Add Route " << (*i) << "/64 via " << (uint32_t) bce->GetTunnelIfIndex ());
      staticRouting->AddNetworkRouteTo ((*i), Ipv6Prefix (64), bce->GetTunnelIfIndex ());
    }
  return true;
}

void Pmipv6Lma::ClearTunnelAndRouting (BindingCache::Entry *bce)
{
  NS_LOG_FUNCTION (this << bce);
  
  // Remove the routes setup for the tunnel.
  Ipv6StaticRoutingHelper staticRoutingHelper;
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  Ptr<Ipv6StaticRouting> staticRouting = staticRoutingHelper.GetStaticRouting (ipv6);
  
  std::list<Ipv6Address> hnpList = bce->GetHomeNetworkPrefixes ();
  for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
    {
      NS_LOG_LOGIC ("Add Route " << (*i) << "/64 via " << (uint32_t)bce->GetTunnelIfIndex ());
      staticRouting->RemoveRoute ((*i), Ipv6Prefix (64), bce->GetTunnelIfIndex (), (*i));
    }
    
  // Remove tunnel device.
  Ptr<Ipv6TunnelL4Protocol> th = GetNode ()->GetObject<Ipv6TunnelL4Protocol> ();
  NS_ASSERT (th);
  th->RemoveTunnel (bce->GetProxyCoa ());
  bce->SetTunnelIfIndex (-1);
}

bool Pmipv6Lma::ModifyTunnelAndRouting (BindingCache::Entry *bce)
{
  NS_LOG_FUNCTION (this << bce);
  uint16_t oldTunnelIf = -1;
  
  Ptr<Ipv6TunnelL4Protocol> th = GetNode ()->GetObject<Ipv6TunnelL4Protocol> ();
  NS_ASSERT (th);
  Ipv6StaticRoutingHelper staticRoutingHelper;
  Ptr<Ipv6> ipv6 = GetNode ()->GetObject<Ipv6> ();
  Ptr<Ipv6StaticRouting> staticRouting = staticRoutingHelper.GetStaticRouting (ipv6);
  
  oldTunnelIf = bce->GetTunnelIfIndex ();
  uint16_t tunnelIf = th->ModifyTunnel (bce->GetOldProxyCoa (), bce->GetProxyCoa ());
  
  // Change routes only if the new tunnel device is different.
  if (oldTunnelIf != tunnelIf)
    {
      std::list<Ipv6Address> hnpList = bce->GetHomeNetworkPrefixes ();
      bce->SetTunnelIfIndex (tunnelIf);
      for (std::list<Ipv6Address>::iterator i = hnpList.begin (); i != hnpList.end (); i++)
        {
          NS_LOG_LOGIC ("Modify Route " << (*i) << "/64 via " << (uint32_t)oldTunnelIf << " to " << tunnelIf);
          staticRouting->RemoveRoute ((*i), Ipv6Prefix (64), oldTunnelIf, (*i));
          staticRouting->AddNetworkRouteTo ((*i), Ipv6Prefix (64), tunnelIf);
        }
    }
  return true;
}

void Pmipv6Lma::DoDelayedRegistration (BindingCache::Entry *bce)
{
  NS_LOG_FUNCTION (this << bce);
  
  BindingCache::Entry *bce_temp = bce->GetTentativeEntry ();
  NS_ASSERT (bce_temp);
  bce->SetTentativeEntry (0);
  
  NS_LOG_LOGIC ("Delayed registration " << bce << " with temp BCE " << bce_temp);
  
  bce->SetProxyCoa (bce_temp->GetProxyCoa ());
  bce->SetAccessTechnologyType (bce_temp->GetAccessTechnologyType ());
  bce->SetHandoffIndicator (bce_temp->GetHandoffIndicator ());
  bce->SetMagLinkAddress (bce_temp->GetMagLinkAddress ());
  bce->SetLastBindingUpdateTime (bce_temp->GetLastBindingUpdateTime ());
  bce->SetReachableTime (bce_temp->GetReachableTime ());
  bce->SetLastBindingUpdateSequence (bce_temp->GetLastBindingUpdateSequence ());  

  delete bce_temp;
  
  ModifyTunnelAndRouting (bce);

  bce->MarkReachable ();
  // Start lifetime timer
  bce->StopReachableTimer ();
  bce->StartReachableTimer ();
    
  // Send PBA
  Ptr<Packet> pktPba;
  pktPba = BuildPba (bce, Ipv6MobilityHeader::BA_STATUS_BINDING_UPDATE_ACCEPTED);
  SendMessage (pktPba, bce->GetProxyCoa (), 64);
}

} /* namespace ns3 */

