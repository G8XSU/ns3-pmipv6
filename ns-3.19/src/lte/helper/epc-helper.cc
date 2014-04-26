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

#include <ns3/epc-helper.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/ipv4-address.h>
#include "ns3/pcap-file-wrapper.h"
#include "ns3/lte-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcHelper")
  ;

NS_OBJECT_ENSURE_REGISTERED (EpcHelper)
  ;


EpcHelper::EpcHelper () 
{
  NS_LOG_FUNCTION (this);
}

EpcHelper::~EpcHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
EpcHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EpcHelper")
    .SetParent<Object> ()
    ;
  return tid;
}

void
EpcHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

void
EpcHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type LteNetDevice.
  //
  Ptr<LteNetDevice> device = nd->GetObject<LteNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("EpcHelper::EnablePcapInternal(): Device " << device << " not of type ns3::LteNetDevice");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, PcapHelper::DLT_PPP);
  pcapHelper.HookDefaultSink<LteNetDevice> (device, "Sniffer", file);
}

} // namespace ns3
