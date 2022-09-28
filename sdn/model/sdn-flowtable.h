#ifndef SDN_FLOWTABLE_H
#define SDN_FLOWTABLE_H

#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"


namespace ns3 {

namespace sdn {

class FlowTableItem
{
public:
  FlowTableItem();
  void Add(Ptr<NetDevice>,Ipv4Address);
  void Delete();
  bool IsEmpty()const;
  int GetHop()const;
  Ipv4Address GetAddress(int)const;
  Ptr<NetDevice> GetNetDevice(int)const;
  void SetTime(Time);
  Time GetTime()const;

private:
  std::vector<std::pair<Ptr<NetDevice>,Ipv4Address>> m_flowpath;
  Time m_time;
};

class FlowTable
{
public:
  FlowTable();
  bool IsExist(Ipv4Address,Ipv4Address)const;
  void Add(Ipv4Address,Ipv4Address,FlowTableItem);
  FlowTableItem Find(Ipv4Address,Ipv4Address)const;
  void Update(Ipv4Address,Ipv4Address,FlowTableItem);
  void Delete(Ipv4Address,Ipv4Address);

private:
  std::map<std::pair<Ipv4Address,Ipv4Address>,FlowTableItem> m_flowtable;
};




}



}








#endif
