/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 CTTC
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Authors: 
 *   Nicola Baldo <nbaldo@cttc.es> (the EpcTftClassifier class)
 *   Giuseppe Piro <g.piro@poliba.it> (part of the code in EpcTftClassifier::Classify () 
 *       which comes from RrcEntity::Classify of the GSoC 2010 LTE module)
 *
 */




#include "epc-tft-classifier.h"
#include "epc-tft.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/tcp-l4-protocol.h"

NS_LOG_COMPONENT_DEFINE ("EpcTftClassifier");

namespace ns3 {

EpcTftClassifier::EpcTftClassifier ()
{
  NS_LOG_FUNCTION (this);
}

void
EpcTftClassifier::Add (Ptr<EpcTft> tft, uint32_t id)
{
  NS_LOG_FUNCTION (this << tft);
  
  m_tftMap[id] = tft;  
  
  // simple sanity check: there shouldn't be more than 16 bearers (hence TFTs) per UE
  NS_ASSERT (m_tftMap.size () <= 16);
}

void
EpcTftClassifier::Delete (uint32_t id)
{
  NS_LOG_FUNCTION (this << id);
  m_tftMap.erase (id);
}
 
uint32_t 
EpcTftClassifier::Classify (Ptr<Packet> p, EpcTft::Direction direction)
{
  NS_LOG_FUNCTION (this << p << direction);

  Ptr<Packet> pCopy = p->Copy ();

  uint8_t byteBuffer[1];
  pCopy->CopyData (byteBuffer, 1);
  uint8_t version = byteBuffer[0] >> 4;
  NS_LOG_INFO ("Version: " << int (version));

  Ipv4Address ipv4LocalAddress;
  Ipv4Address ipv4RemoteAddress;
  Ipv6Address ipv6LocalAddress;
  Ipv6Address ipv6RemoteAddress;
  uint8_t protocol;
  uint8_t tos;
  uint16_t localPort = 0;
  uint16_t remotePort = 0;

  switch (version)
    {
    case 4: // Ipv4
      {
        Ipv4Header ipv4Header;
        pCopy->RemoveHeader (ipv4Header);

        if (direction ==  EpcTft::UPLINK)
          {
            ipv4LocalAddress = ipv4Header.GetSource ();
            ipv4RemoteAddress = ipv4Header.GetDestination ();
          }
        else
          {
            NS_ASSERT (direction ==  EpcTft::DOWNLINK);
            ipv4RemoteAddress = ipv4Header.GetSource ();
            ipv4LocalAddress = ipv4Header.GetDestination ();
          }
        protocol = ipv4Header.GetProtocol ();
        tos = ipv4Header.GetTos ();
        break;
      }
    case 6: // Ipv6
      {
        Ipv6Header ipv6Header;
        pCopy->RemoveHeader (ipv6Header);
        if (direction ==  EpcTft::UPLINK)
          {
            ipv6LocalAddress = ipv6Header.GetSourceAddress ();
            ipv6RemoteAddress = ipv6Header.GetDestinationAddress ();
          }
        else
          {
            NS_ASSERT (direction ==  EpcTft::DOWNLINK);
            ipv6RemoteAddress = ipv6Header.GetSourceAddress ();
            ipv6LocalAddress = ipv6Header.GetDestinationAddress ();
          }
        protocol = ipv6Header.GetNextHeader ();
        tos = ipv6Header.GetTrafficClass ();
      }
      break;
    default:
      NS_ASSERT_MSG (false, "Ip version not supported.");
      break; // never reached.
    }

  if (protocol == UdpL4Protocol::PROT_NUMBER)
    {
      UdpHeader udpHeader;
      pCopy->RemoveHeader (udpHeader);

      if (direction == EpcTft::UPLINK)
        {
          localPort = udpHeader.GetSourcePort ();
          remotePort = udpHeader.GetDestinationPort ();
        }
      else
        {
          remotePort = udpHeader.GetSourcePort ();
          localPort = udpHeader.GetDestinationPort ();
        }
    }
  else if (protocol == TcpL4Protocol::PROT_NUMBER)
    {
      TcpHeader tcpHeader;
      pCopy->RemoveHeader (tcpHeader);
      if (direction == EpcTft::UPLINK)
        {
          localPort = tcpHeader.GetSourcePort ();
          remotePort = tcpHeader.GetDestinationPort ();
        }
      else
        {
          remotePort = tcpHeader.GetSourcePort ();
          localPort = tcpHeader.GetDestinationPort ();
        }
    }
  else
    {
      NS_LOG_INFO ("Unknown protocol: " << protocol);
      return 0;  // no match
    }

  switch (version)
    {
    case 4:
      NS_LOG_INFO ("Classifing packet:"
                     << " localAddr="  << ipv4LocalAddress
                     << " remoteAddr=" << ipv4RemoteAddress
                     << " localPort="  << localPort
                     << " remotePort=" << remotePort
                     << " tos=0x" << (uint16_t) tos );
      break;
    case 6:
      NS_LOG_INFO ("Classifing packet:"
                     << " localAddr="  << ipv6LocalAddress
                     << " remoteAddr=" << ipv6RemoteAddress
                     << " localPort="  << localPort
                     << " remotePort=" << remotePort
                     << " tos=0x" << (uint16_t) tos );
      break;
    }


    // now it is possible to classify the packet!
    // we use a reverse iterator since filter priority is not implemented properly.
    // This way, since the default bearer is expected to be added first, it will be evaluated last.
    std::map <uint32_t, Ptr<EpcTft> >::const_reverse_iterator it;
    NS_LOG_LOGIC ("TFT MAP size: " << m_tftMap.size ());

    for (it = m_tftMap.rbegin (); it != m_tftMap.rend (); ++it)
      {
        NS_LOG_LOGIC ("TFT id: " << it->first );
        NS_LOG_LOGIC (" Ptr<EpcTft>: " << it->second);
        Ptr<EpcTft> tft = it->second;
        switch (version)
          {
          case 4:
            if (tft->Matches (direction, ipv4RemoteAddress, ipv4LocalAddress, remotePort, localPort, tos))
              {
                NS_LOG_LOGIC ("matches with TFT ID = " << it->first);
                return it->first; // the id of the matching TFT
              }
            break;
          case 6:
            if (tft->Matches (direction, ipv6RemoteAddress, ipv6LocalAddress, remotePort, localPort, tos))
              {
                NS_LOG_LOGIC ("matches with TFT ID = " << it->first);
                return it->first; // the id of the matching TFT
              }
            break;
          }

      }
    NS_LOG_LOGIC ("no match");
    return 0;  // no match

}
} // namespace ns3
