#include "sdn-flowtable.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SDNFLOWTABLE");

namespace sdn {

FlowTableItem::FlowTableItem()
    : m_time (Seconds(0))
{

}

void
FlowTableItem::Add(Ptr<NetDevice> dev, Ipv4Address add)
{
  m_flowpath.push_back({dev,add});
}

void
FlowTableItem::Delete()
{
  m_flowpath.clear();
}

bool
FlowTableItem::IsEmpty()const
{
  return m_flowpath.empty();
}

int
FlowTableItem::GetHop()const
{
  return m_flowpath.size();
}

Ipv4Address
FlowTableItem::GetAddress(int i)const
{
  return m_flowpath[i].second;
}

Ptr<NetDevice>
FlowTableItem::GetNetDevice(int i)const
{
  return m_flowpath[i].first;
}

void
FlowTableItem::SetTime(Time time)
{
  m_time = time;
}

Time
FlowTableItem::GetTime()const
{
  return m_time;
}

FlowTable::FlowTable()
{
}

bool
FlowTable::IsExist(Ipv4Address src, Ipv4Address dst)const
{
  return m_flowtable.count({src,dst});
}

void
FlowTable::Add(Ipv4Address src, Ipv4Address dst, FlowTableItem fti)
{
  m_flowtable[{src,dst}] = fti;
}

FlowTableItem
FlowTable::Find(Ipv4Address src, Ipv4Address dst)const
{
  return m_flowtable.find({src,dst})->second;
}

void
FlowTable::Update(Ipv4Address src, Ipv4Address dst, FlowTableItem fti)
{
  m_flowtable[{src,dst}] = fti;
}

void
FlowTable::Delete(Ipv4Address src, Ipv4Address dst)
{
  m_flowtable.erase({src,dst});
}


}

}
