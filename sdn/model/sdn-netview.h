#ifndef SDN_NETVIEW_H
#define SDN_NETVIEW_H

#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "sdn-flow-table.h"


namespace ns3 {

namespace sdn {

struct Edge
{
	double load = 0;
};

class ControlCenter
{
public:
	void SetController(int);
	void AddSwitchToController(int,int);
	Time CalculateDelay(int);
	void RecvRREQ(int,int,int);
	bool IsController(int)const;
	bool IsExistPath(int,int)const;

private:
	std::map<std::pair<int,int>,Edge> m_edges;
	std::vector<std::vector<int>> m_G;
	std::map<int,std::vector<int>> m_controllers;
	std::map<int,int> m_swcTocon;

	//the nodes exist in the 'm_path' means these
	//nodes have been known the routes to transmit the flow
	std::map<std::pair<int,int>,std::vector<int>> m_path;
};





}



}





#endif
