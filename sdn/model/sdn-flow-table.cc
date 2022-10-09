#include "sdn-flow-table.h"

namespace ns3 {

namespace sdn {

//FlowTableEntry::FlowTableEntry()
//		 {
//
//		 }
////FlowTableEntry::FlowTableEntry(FlowLevel lv)
////{
////	m_level = lv;
////}
//
//void
//FlowTableEntry::SetSource(Ipv4Address add)
//{
//	m_src = add;
//}
//
//void
//FlowTableEntry::SetDestination(Ipv4Address add)
//{
//	m_dst = add;
//}
//
//void
//FlowTableEntry::SetGateWay(Ipv4Address add)
//{
//	m_gateway = add;
//}
//
////void
////FlowTableEntry::SetFlowLevel(FlowLevel lv)
////{
////	m_level = lv;
////}
//
//void
//FlowTableEntry::SetOutputDevice(Ptr<NetDevice> dev)
//{
//	m_dev = dev;
//}
//
//Ipv4Address
//FlowTableEntry::GetSource(void)const
//{
//	return m_src;
//}
//
//Ipv4Address
//FlowTableEntry::GetDestination(void)const
//{
//	return m_dst;
//}
//
//Ipv4Address
//FlowTableEntry::GetGateWay(void)const
//{
//	return m_gateway;
//}
//
////FlowTableEntry::FlowLevel
////FlowTableEntry::GetFlowlevel(void)const
////{
////	return m_level;
////}
//
//Ptr<NetDevice>
//FlowTableEntry::GetOutputDevice(void)const
//{
//	return m_dev;
//}

FlowTable::FlowTable()
{

}

bool
FlowTable::IsExist(Ipv4Address src, Ipv4Address dst)const
{
	return m_table.count({src,dst});
}

void
FlowTable::Add(Ipv4Address src, Ipv4Address dst, Ptr<Ipv4Route> fte)
{
	m_table[{src,dst}] = fte;
}

void
FlowTable::Add(Ipv4Address src, Ipv4Address dst, Ipv4Route fte)
{
	Ptr<Ipv4Route> rte = Create<Ipv4Route>();
	rte->SetSource(fte.GetSource());
	rte->SetDestination(fte.GetDestination());
	rte->SetGateway(fte.GetGateway());
	rte->SetOutputDevice(fte.GetOutputDevice());
	m_table[{src,dst}] = rte;
}

Ptr<Ipv4Route>
FlowTable::Get(Ipv4Address src, Ipv4Address dst)
{
	return m_table[{src,dst}];
}

void
FlowTable::Delete(Ipv4Address src, Ipv4Address dst)
{
	if(IsExist(src,dst))
	{
		m_table.erase({src,dst});
	}
}

}

}
