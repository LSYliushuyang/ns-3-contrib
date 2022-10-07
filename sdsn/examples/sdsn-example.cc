/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/sdsn-module.h"

#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"

using namespace ns3;


int 
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);

  cmd.Parse (argc,argv);

  /* ... */

  NodeContainer c;
  c.Create(2);

  PointToPointWirelessHelper p2p;
  p2p.Install(c);

  SdsnHelper sdn;
  InternetStackHelper internet;
  internet.SetRoutingHelper(sdn);
  internet.Install(c);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0","255.255.255.0");
  NetDeviceContainer devs;
  devs.Add(c.Get(0)->GetDevice(0));
  devs.Add(c.Get(0)->GetDevice(1));
  address.Assign(devs);

  Ipv4AddressHelper address1;
  address1.SetBase("10.2.2.0","255.255.255.0");
  NetDeviceContainer devs1;
  devs1.Add(c.Get(1)->GetDevice(0));
  devs1.Add(c.Get(1)->GetDevice(1));
  address1.Assign(devs1);

//  OnOffHelper onoff("ns3::UdpSocketFactory",InetSocketAddress(c.Get(0)->GetObject<Ipv4L3Protocol>()->GetAddress(0,1).GetAddress(),9));
//  onoff.SetAttribute("PacketSize",  UintegerValue (512));
//  onoff.SetAttribute("OnTime",StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
//  onoff.SetAttribute("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
//  onoff.SetAttribute("MaxBytes", UintegerValue (0));
//  ApplicationContainer onoffClientApps = onoff.Install(c.Get(0));
//  onoffClientApps.Start(Seconds(1.0));
//  onoffClientApps.Stop(Seconds(10));
//
//  PacketSinkHelper sink("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),9));
//  ApplicationContainer sinkApps = sink.Install(c.Get(1));


  Simulator::Stop(Seconds(20));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


