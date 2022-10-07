#ifndef SDN_ID_LOOKUP_TABLE_H
#define SDN_ID_LOOKUP_TABLE_H

#include "ns3/ipv4-address.h"
#include "ns3/node.h"

namespace ns3 {

namespace sdn {

class LookupTable
{
public:
  LookupTable();
  bool IsExistSeq(uint32_t)const;
  bool IsExistAdd(Ipv4Address)const;
  int FindNAdd(uint32_t)const;
  Ipv4Address FindAdd(uint32_t,int)const;
  uint32_t FindSeqNo(Ipv4Address)const;
  void Add(uint32_t,Ipv4Address);
  void Delete(uint32_t,Ipv4Address);

private:
  std::map<uint32_t,std::vector<Ipv4Address>> m_seqNo_To_Add;
  std::map<Ipv4Address,uint32_t> m_Add_To_seqNo;
//  std::map<Ptr<Node>,uint32_t> m_Node_To_seqNo;
//  std::map<uint32_t,Ptr<Node>> m_seqNo_To_Node;

};








}




}









#endif
