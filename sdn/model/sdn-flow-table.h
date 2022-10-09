#ifndef SDN_FLOW_TABLE_H
#define SDN_FLOW_TABLE_H

#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"
#include "ns3/ipv4-route.h"

namespace ns3 {

namespace sdn {

//class FlowTableEntry
//{
//public:
////	enum FlowLevel {
////		MANAGEMENT,
////		REALTIME,
////		MASSIVEDATA,
////		STREAMINGMEDIA,
////		NORMAL};
//	FlowTableEntry();
////	FlowTableEntry(FlowLevel);
//
//	void SetSource(Ipv4Address);
//	void SetDestination(Ipv4Address);
//	void SetGateWay(Ipv4Address);
////	void SetFlowLevel(FlowLevel);
//	void SetOutputDevice(Ptr<NetDevice>);
//
//	Ipv4Address GetSource(void)const;
//	Ipv4Address GetDestination(void)const;
//	Ipv4Address GetGateWay(void)const;
////	FlowLevel GetFlowlevel(void)const;
//	Ptr<NetDevice> GetOutputDevice(void)const;
//
//private:
//
//	Ipv4Address m_src;
//	Ipv4Address m_dst;
//	Ipv4Address m_gateway;
////	FlowLevel m_level;
//	Ptr<NetDevice> m_dev;
//
//};

class FlowTable
{
public:
	FlowTable();
	bool IsExist(Ipv4Address,Ipv4Address)const;
//	int Count(Ipv4Address,Ipv4Address)const;
	void Add(Ipv4Address,Ipv4Address,Ptr<Ipv4Route>);
	void Add(Ipv4Address,Ipv4Address,Ipv4Route);
	Ptr<Ipv4Route> Get(Ipv4Address,Ipv4Address);
	void Delete(Ipv4Address,Ipv4Address);

private:
	std::map<std::pair<Ipv4Address,Ipv4Address>,Ptr<Ipv4Route>> m_table;

};




}

}



#endif
