#ifndef SDN_NETVIEW_H
#define SDN_NETVIEW_H

#include "ns3/net-device.h"
#include "ns3/nstime.h"


namespace ns3 {

namespace sdn {

class Edge
{
public:
  Edge();
  double GetLoad()const;
  Time GetDelay()const;
  void SetLoad();
  void SetDelay();

private:
  double m_load;
  Time m_delay;
};


class SeqPath
{
public:
  SeqPath();
  int GetHop()const;
  uint32_t GetSeq(int)const;

private:
  std::vector<uint32_t> m_path;
  int m_hop;
};

class NetView
{
public:
  NetView();
  Edge GetEdge(int,int)const;
  void UpdateLoad(int,int,double);
  void UpdateDelay(int,int,Time);

private:
  std::vector<std::vector<Edge>> m_view;
  int m_num;
};





}



}





#endif
