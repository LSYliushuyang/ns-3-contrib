#include "sdn-netview.h"

namespace ns3 {

extern std::map<Ptr<Node>,int> NODETOIND;
extern std::map<int,Ptr<Node>> INDTONODE;
extern std::map<Ipv4Address,int> ADDTOIND;

namespace sdn {

class RoutingProtocol;


void
ControlCenter::SetController(int i)
{
  if(!IsController(i))
    {
      std::vector<int> temp;
      temp.push_back(i);
      m_controllers[i] = temp;
      m_swcTocon[i] = i;
    }
}

void
ControlCenter::AddSwitchToController(int swc, int con)
{
  if(!IsController(con)) return;
  if(IsController(swc)) return;
  for(auto it = m_controllers[con].begin();
      it != m_controllers[con].end();
      ++it)
    {
      if(*it == swc) return;
    }
  m_controllers[con].push_back(swc);
  m_swcTocon[swc] = con;
  return;
}

Time
ControlCenter::CalculateDelay(int swc)
{
  int con = m_swcTocon[swc];
  Ptr<Node> swc_n = INDTONODE.find(swc)->second;
  Ptr<Node> con_n = INDTONODE.find(con)->second;
  /*
   *
   *
   *
   *
   */

  return Seconds(0);
}

void
ControlCenter::RecvRREQ(int req,Ipv4Address src ,Ipv4Address dst)
{
  //RREQ is from 'req'
  //To request req's route of the flow from 'src' to 'dst'

  //the nodes on the flow between 'src' and 'dst' have know route
  //but a RREQ generated
	int src_ind = ADDTOIND.find(src)->second;
	int dst_ind = ADDTOIND.find(dst)->second;
  if(IsExistPath(src_ind,dst_ind))
    {
      for(auto it = m_path[{src_ind,dst_ind}].begin();
          it != m_path[{src_ind,dst_ind}].end();
              ++it)
        {
          if(*it == req)
            {
              //the node on the path do not know the route
              /*
               *
               */
              return;
            }
        }
      //the node 'req' is not on the path
      //exist a node on the path before 'req' transmit to the wrong way
      /*
       *
       */
      return;
    }
  //A new flow request from 'src' to 'dst', requested by 'req'
  // 'req' == 'src'
  std::vector<int> path = CalculatePath(src_ind,dst_ind);
  Ptr<Node> no;
  Ptr<RoutingProtocol> rp;
  Time time;
  for(auto it = path.begin(); it != path.end()-1; ++it)
  {
	  no = INDTONODE.find(*it)->second;
	  rp = no->GetObject<RoutingProtocol>();
	  time = CalculateDelay(*it);
	  Simulator::Schedule(time,&RoutingProtocol::RecvRREP,rp,src,dst,*(it+1));
  }

  return;
}





}



}
