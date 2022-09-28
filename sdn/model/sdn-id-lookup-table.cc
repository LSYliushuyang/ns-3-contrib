#include "sdn-id-lookup-table.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SDNIDLOOKUPTABLE");

namespace sdn {

LookupTable::LookupTable()
{
}

bool
LookupTable::IsExistSeq(uint32_t i )const
{
  return m_seqNo_To_Add.count(i);
}

bool
LookupTable::IsExistAdd(Ipv4Address add)const
{
  return m_Add_To_seqNo.count(add);
}

int
LookupTable::FindNAdd(uint32_t i )const
{
  return m_seqNo_To_Add.find(i)->second.size();
}

Ipv4Address
LookupTable::FindAdd(uint32_t seq, int i)const
{
  return m_seqNo_To_Add.find(seq)->second[i];
}

uint32_t
LookupTable::FindSeqNo(Ipv4Address add)const
{
  return m_Add_To_seqNo.find(add)->second;
}

void
LookupTable::Add(uint32_t seq, Ipv4Address add)
{
  if(this->IsExistSeq(seq))
    {
      m_seqNo_To_Add.find(seq)->second.push_back(add);
    }
  else
    {
      std::vector<Ipv4Address> temp;
      temp.push_back(add);
      m_seqNo_To_Add[seq] = temp;
    }
  m_Add_To_seqNo[add] = seq;
}

void
LookupTable::Delete(uint32_t seq, Ipv4Address add)
{
  m_Add_To_seqNo.erase(add);
  for(auto it = m_seqNo_To_Add[seq].begin(); it != m_seqNo_To_Add[seq].end(); ++it)
    {
      if(*it == add)
        {
          m_seqNo_To_Add[seq].erase(it);
          break;
        }
    }
  if(m_seqNo_To_Add[seq].empty())
    m_seqNo_To_Add.erase(seq);
}


}



}
