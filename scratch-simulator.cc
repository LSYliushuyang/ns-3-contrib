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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/sdn.h"
#include "ns3/sdn-helper.h"
#include "ns3/config-store.h"

extern std::map<ns3::Ptr<ns3::Node>,int> NODETOIND;
extern std::map<int,ns3::Ptr<ns3::Node>> INDTONODE;
extern std::map<ns3::Ipv4Address,int> ADDTOIND;


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ScratchSimulator");

int num = 0;

void TrafficControlDropCallback(std::string context,Ptr<const QueueDiscItem> qdi,const char * reason)
{
	num = num +1;
	NS_LOG_UNCOND("DropAfterDequeue" <<reason<< context);
}


void MacTxCallback(std::string context,Ptr<const Packet> p)
{
	NS_LOG_UNCOND("Tx"
			<< context);
}

void MacRxCallback(std::string context,Ptr<const Packet> p)
{
	NS_LOG_UNCOND("Drop"
			<< context);
}

void TxRxPointToPointCallback(std::string context,Ptr<const Packet> p, Ptr<NetDevice> d1, Ptr<NetDevice> d2, Time t1, Time t2)
{
	NS_LOG_UNCOND("TxRx"<< "at " <<t1 << "&" <<t2<< context);
}




int 
main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("Scratch Simulator");

  NodeContainer c;
  c.Create(9);

  for(int i = 0; i < 9; ++i)
  {
	  NODETOIND[c.Get(i)] = i;
	  INDTONODE[i] = c.Get(i);
  }

  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::ListPositionAllocator");

  mobility.SetMobilityModel("ns3::SphericalPositionMobilityModel");

  Ptr<PositionAllocator> position = mobility.GetPositionAllocator();
  position->GetObject<ListPositionAllocator>()->Add(Vector(0,0,6371e3 + 780000));			//0,0
  position->GetObject<ListPositionAllocator>()->Add(Vector(M_PI/3,M_PI,6371e3 + 780000));	//0,1
  position->GetObject<ListPositionAllocator>()->Add(Vector(-M_PI/3,M_PI,6371e3 + 780000));	//0,2

  position->GetObject<ListPositionAllocator>()->Add(Vector(0,M_PI/3,6371e3 + 780000));			//1,0
  position->GetObject<ListPositionAllocator>()->Add(Vector(M_PI/3,-M_PI*2/3,6371e3 + 780000));	//1,1
  position->GetObject<ListPositionAllocator>()->Add(Vector(-M_PI/3,-M_PI*2/3,6371e3 + 780000));	//1,2

  position->GetObject<ListPositionAllocator>()->Add(Vector(0,M_PI*2/3,6371e3 + 780000));			//2,0
  position->GetObject<ListPositionAllocator>()->Add(Vector(M_PI/3,-M_PI/3,6371e3 + 780000));	//2,1
  position->GetObject<ListPositionAllocator>()->Add(Vector(-M_PI/3,-M_PI/3,6371e3 + 780000));	//2,2

  mobility.Install(c);

  c.Get(0)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(3.14/2/10,0.0,0.0));
  c.Get(1)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(-3.14/2/10,0.0,0.0));
  c.Get(2)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(-3.14/2/10,0.0,0.0));
  c.Get(3)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(3.14/2/10,0.0,0.0));
  c.Get(4)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(-3.14/2/10,0.0,0.0));
  c.Get(5)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(-3.14/2/10,0.0,0.0));
  c.Get(6)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(3.14/2/10,0.0,0.0));
  c.Get(7)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(-3.14/2/10,0.0,0.0));
  c.Get(8)->GetObject<SphericalPositionMobilityModel>()->SetVelocity(Vector(-3.14/2/10,0.0,0.0));

  sdn::RoutingProtocol::NETCENTER.SetNum(9);
  sdn::RoutingProtocol::NETCENTER.InitG();


  PointToPointWirelessHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("1.9Mbps"));
  for(int i = 0; i < 3; ++i)
  {
	  for(int j = 0; j < 3; ++j)
	  {
		  NodeContainer temp;
		  temp.Add(c.Get(i*3+j));
		  temp.Add(c.Get(j+1>=3? i*3+j+1-3: i*3+j+1));

		  NetDeviceContainer ndc;
		  ndc = p2p.Install(temp);

		  sdn::Edge edge;
		  edge.delay = ndc.Get(0)->GetChannel()->GetDelay();
//		  sdn::RoutingProtocol::NETCENTER.ChangeEdge(i*3+j,j+1>=3? i*3+j+1-3: i*3+j+1,edge);
//		  sdn::RoutingProtocol::NETCENTER.ChangeEdge(j+1>=3? i*3+j+1-3: i*3+j+1,i*3+j,edge);
//
//		  sdn::RoutingProtocol::NETCENTER.ChangeG(i*3+j,j+1>=3? i*3+j+1-3: i*3+j+1,1);
//		  sdn::RoutingProtocol::NETCENTER.ChangeG(j+1>=3? i*3+j+1-3: i*3+j+1,i*3+j,1);
	  }
  }

  for(int i = 0; i < 2; ++i)
  {
	  for(int j = 0; j < 3; ++j)
	  {
		  NodeContainer temp;
		  temp.Add(c.Get(i*3+j));
		  temp.Add(c.Get(i*3+j+3));

		  NetDeviceContainer ndc;
		  ndc = p2p.Install(temp);

		  sdn::Edge edge;
		  edge.delay = ndc.Get(0)->GetChannel()->GetDelay();
//		  sdn::RoutingProtocol::NETCENTER.ChangeEdge(i*3+j,i*3+j+3,edge);
//		  sdn::RoutingProtocol::NETCENTER.ChangeEdge(i*3+j+3,i*3+j,edge);
//
//		  sdn::RoutingProtocol::NETCENTER.ChangeG(i*3+j,i*3+j+3,1);
//		  sdn::RoutingProtocol::NETCENTER.ChangeG(i*3+j+3,i*3+j,1);
	  }
  }


  sdn::RoutingProtocol::NETCENTER.Init(c);
  sdn::RoutingProtocol::NETCENTER.SetController(4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(0,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(1,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(2,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(3,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(4,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(5,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(6,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(7,4);
  sdn::RoutingProtocol::NETCENTER.AddSwitchToController(8,4);



  SDNHelper sdnh;

  InternetStackHelper it;
  it.SetRoutingHelper(sdnh);
  it.Install(c);

  Ipv4AddressHelper ip;
  ip.SetBase("10.1.1.0","255.255.255.0");



  NetDeviceContainer ndc;
  for(int i =0; i < 3; ++i)
  {
	  for(int j = 0; j < 3; ++j)
	  {
		  for(int k = 0; k < int(c.Get(i*3+j)->GetNDevices()); ++k)
		  {
			  if(!c.Get(i*3+j)->GetDevice(k)->IsPointToPoint()) continue;
			  ndc.Add(c.Get(i*3+j)->GetDevice(k));
		  }
	  }
  }

  ip.Assign(ndc);

  for(int i = 0; i < 9; ++i)
  {
	  uint32_t interface = c.Get(i)->GetObject<Ipv4>()->GetNInterfaces();
	  for(uint32_t it = 0; it < interface; ++it)
	  {
		  ADDTOIND[c.Get(i)->GetObject<Ipv4>()->GetAddress(it, 0).GetLocal()] = i;
	  }
  }







  Ipv4Address add = c.Get(5)->GetObject<Ipv4L3Protocol>()->GetAddress(1,0).GetAddress();
  OnOffHelper onoff("ns3::UdpSocketFactory",InetSocketAddress(add,9));
  onoff.SetAttribute("PacketSize",  UintegerValue (512));
  onoff.SetAttribute("OnTime",StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  onoff.SetAttribute("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
  onoff.SetAttribute("DataRate", StringValue("2Mbps"));
  onoff.SetAttribute("MaxBytes", UintegerValue (0));
  ApplicationContainer onoffClientApps = onoff.Install(c.Get(0));
  onoffClientApps.Start(Seconds(1.0));
  onoffClientApps.Stop(Seconds(10));

  PacketSinkHelper sink("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),9));
  ApplicationContainer sinkApps = sink.Install(c.Get(5));
  sinkApps.Start(Seconds(1));
  sinkApps.Stop(Seconds(15));




  LogComponentEnable("OnOffApplication",LOG_INFO);
  LogComponentEnable("PacketSink",LOG_INFO);

//  	  LogComponentEnable("SDN",LOG_DEBUG );
//
  	  Config::Connect(
  			  "/$ns3::NodeListPriv/NodeList/*/$ns3::Node/"
  			  "$ns3::TrafficControlLayer/RootQueueDiscList/*/"
  			  "$ns3::FqCoDelQueueDisc/DropBeforeEnqueue",
			  MakeCallback(&TrafficControlDropCallback));

  	  Config::Set("/$ns3::NodeListPriv/NodeList/0/$ns3::Node/"
  			  "$ns3::TrafficControlLayer/RootQueueDiscList/1/"
  			  "$ns3::FqCoDelQueueDisc/Target",StringValue("5s"));
  	  Config::Set("/$ns3::NodeListPriv/NodeList/0/$ns3::Node/"
  			  "$ns3::TrafficControlLayer/RootQueueDiscList/1/"
  			  "$ns3::FqCoDelQueueDisc/MaxSize",StringValue("1p"));
//
// 	  Config::Connect(
//  			  "/NodeList/2/DeviceList/*/$ns3::PointToPointWirelessNetDevice/MacRx",
//			  MakeCallback(&MacRxCallback));

  Config::SetDefault("ns3::ConfigStore::Filename",StringValue("sdn.txt"));
  Config::SetDefault("ns3::ConfigStore::FileFormat",StringValue("RawText"));
  Config::SetDefault("ns3::ConfigStore::Mode",StringValue("Save"));
  ConfigStore output;
//  output.ConfigureDefaults();
  output.ConfigureAttributes();






  Simulator::Stop(Seconds(15));
  Simulator::Run ();
  Simulator::Destroy ();

  std::cout<<"TOTAL DROP NUMBER: " << num <<std::endl;

  return 0;
}
