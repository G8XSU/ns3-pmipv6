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

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"

#include "identifier.h"
#include "ipv6-mobility.h"
#include "ipv6-mobility-demux.h"
#include "ipv6-mobility-header.h"
#include "ipv6-mobility-option.h"
#include "ipv6-mobility-option-demux.h"

#include "pmipv6-agent.h"

NS_LOG_COMPONENT_DEFINE ("Ipv6Mobility");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (Ipv6Mobility);

TypeId Ipv6Mobility::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6Mobility")
    .SetParent<Object>()
	.AddAttribute ("MobilityNumber", "The IPv6 mobility number.",
	               UintegerValue (0),
				   MakeUintegerAccessor (&Ipv6Mobility::GetMobilityNumber),
				   MakeUintegerChecker<uint8_t> ())
	;
  return tid;
}

Ipv6Mobility::~Ipv6Mobility()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void Ipv6Mobility::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  Object::DoDispose ();
}

void Ipv6Mobility::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node> Ipv6Mobility::GetNode () const
{
  NS_LOG_FUNCTION_NOARGS();
  
  return m_node;
}

uint8_t Ipv6Mobility::ProcessOptions(Ptr<Packet> packet, uint8_t offset, uint8_t length, Ipv6MobilityOptionBundle &bundle)
{
  NS_LOG_FUNCTION (this << packet << length);
  Ptr<Packet> p = packet->Copy ();
  p->RemoveAtStart(offset);
  
  Ptr<Ipv6MobilityOptionDemux> ipv6MobilityOptionDemux = GetNode()->GetObject<Ipv6MobilityOptionDemux>();
  NS_ASSERT(ipv6MobilityOptionDemux != 0);
  
  Ptr<Ipv6MobilityOption> ipv6MobilityOption = 0;
  
  uint8_t processedSize = 0;
  uint32_t size = p->GetSize ();
  uint8_t *data = new uint8_t[size];
  p->CopyData (data, size);
  
  uint8_t optType;
  uint8_t optLen;

  while ( processedSize < length )
    {
      optType = *(data + processedSize);
	  
	  ipv6MobilityOption = ipv6MobilityOptionDemux -> GetOption ( optType );
	  
	  if ( ipv6MobilityOption == 0 )
	    {
		  if ( optType == 0 )
		    {
			  optLen = 1;
			}
		  else
		    {
			  optLen = *(data + processedSize + 1) + 2;
			}
			
		  NS_LOG_LOGIC("No matched Ipv6MobilityOption for type=" << (uint32_t)optType );
		}
	  else
	    {
		  optLen = ipv6MobilityOption -> Process (packet, offset + processedSize, bundle);
		}
	  
	  processedSize += optLen;
	  p->RemoveAtStart(optLen);
    }

  delete [] data;
  
  return processedSize;
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityBindingUpdate);

TypeId Ipv6MobilityBindingUpdate::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityBindingUpdate")
    .SetParent<Ipv6Mobility>()
	.AddConstructor<Ipv6MobilityBindingUpdate>()
	;
  return tid;
}

Ipv6MobilityBindingUpdate::~Ipv6MobilityBindingUpdate()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityBindingUpdate::GetMobilityNumber () const
{
  return MOB_NUMBER;
}

uint8_t Ipv6MobilityBindingUpdate::Process (Ptr<Packet> p, Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION_NOARGS();

  Ptr<Packet> packet = p->Copy();
  
  Ipv6MobilityBindingUpdateHeader buh;
  Ipv6MobilityOptionBundle bundle;
  
  /* Proxy Mobile Ipv6 process routine */
  packet->RemoveHeader(buh);

  if(buh.GetFlagP())
    {
      Ptr<Pmipv6Agent> pmipv6Agent = GetNode()->GetObject<Pmipv6Agent>();

      if( pmipv6Agent )
        {
          Simulator::ScheduleNow( &Pmipv6Agent::Receive, pmipv6Agent, p, src, dst, interface);
          return 0;
        }
    }
  
  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = GetNode()->GetObject<Ipv6MobilityDemux>();
  NS_ASSERT( ipv6MobilityDemux );
  
  Ptr<Ipv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility(buh.GetMhType());
  NS_ASSERT( ipv6Mobility );

  uint8_t length = ((buh.GetHeaderLen() + 1 ) << 3) - buh.GetOptionsOffset();
  
  ipv6Mobility->ProcessOptions (packet, buh.GetOptionsOffset(), length, bundle);

  NS_LOG_LOGIC(" No Handler for Binding Update");
  
  return 0;
}

NS_OBJECT_ENSURE_REGISTERED (Ipv6MobilityBindingAck);

TypeId Ipv6MobilityBindingAck::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::Ipv6MobilityBindingAck")
    .SetParent<Ipv6Mobility>()
	.AddConstructor<Ipv6MobilityBindingAck>()
	;
  return tid;
}

Ipv6MobilityBindingAck::~Ipv6MobilityBindingAck()
{
  NS_LOG_FUNCTION_NOARGS ();
}

uint8_t Ipv6MobilityBindingAck::GetMobilityNumber () const
{
  return MOB_NUMBER;
}

uint8_t Ipv6MobilityBindingAck::Process (Ptr<Packet> p, Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION_NOARGS();
  
  Ptr<Packet> packet = p->Copy();
  
  Ipv6MobilityBindingAckHeader bah;
  Ipv6MobilityOptionBundle bundle;
  
  /* Proxy Mobile Ipv6 process routine */
  packet->PeekHeader(bah);
  
  if(bah.GetFlagP())
    {
	  Ptr<Pmipv6Agent> pmipv6Agent = GetNode()->GetObject<Pmipv6Agent>();
	  
	  if( pmipv6Agent )
	    {
		  Simulator::ScheduleNow( &Pmipv6Agent::Receive, pmipv6Agent, p, src, dst, interface);

          return 0;		  
		}
	}
	
  Ptr<Ipv6MobilityDemux> ipv6MobilityDemux = GetNode()->GetObject<Ipv6MobilityDemux>();
  NS_ASSERT( ipv6MobilityDemux );
  
  Ptr<Ipv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility(bah.GetMhType());
  NS_ASSERT( ipv6Mobility );

  uint8_t length = ((bah.GetHeaderLen() + 1 ) << 3) - bah.GetOptionsOffset();
  
  ipv6Mobility->ProcessOptions ( packet, bah.GetOptionsOffset(), length, bundle);

  NS_LOG_LOGIC(" No Handler for Binding Ack");
  
  return 0;
}

} /* namespace ns3 */
