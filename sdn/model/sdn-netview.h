#ifndef SDN_NETVIEW_H
#define SDN_NETVIEW_H

#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "sdn-flow-table.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/sdn.h"

extern int NPLANE;
extern int NPERPLANE;

namespace ns3 {

namespace sdn {

struct Edge
{
	double load = 0;
	Time delay = Seconds(0);
};

class ControlCenter
{
public:
	void SetController(int);
	void AddSwitchToController(int,int);
	Time CalculateDelay(int);
	void RecvRREQ(int,Ipv4Address,Ipv4Address);
	bool IsController(int)const;
	bool IsExistPath(int,int)const;

	std::vector<int> CalculatePath(int,int);

	Ipv4Address GetGateWay(int,int);
	Ptr<NetDevice> GetOutputDevice(int,int);

private:
	std::map<std::pair<int,int>,Edge> m_edges;
	std::vector<std::vector<int>> m_G;		//1 means connected ;0 means myself ;-1 means not connected
	std::map<int,std::vector<int>> m_controllers;
	std::map<int,int> m_swcTocon;

	//the nodes exist in the 'm_path' means these
	//nodes have been known the routes to transmit the flow
	std::map<std::pair<int,int>,std::vector<int>> m_path;
};





}



}





#endif
