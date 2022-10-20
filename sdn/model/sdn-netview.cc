#include "sdn-netview.h"

extern std::map<ns3::Ptr<ns3::Node>,int> NODETOIND;
extern std::map<int,ns3::Ptr<ns3::Node>> INDTONODE;
extern std::map<ns3::Ipv4Address,int> ADDTOIND;

namespace ns3 {



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

	int src_ind;
	if(src.IsInitialized())
		src_ind = ADDTOIND.find(src)->second;
	else
		src_ind = req;

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
  m_path[{src_ind,dst_ind}] = path;
  Ptr<Node> no;
  Ptr<RoutingProtocol> rp;
  Time time;

  if(req == src_ind)
  {
	  src = INDTONODE.find(*path.begin())->second->GetObject<RoutingProtocol>()->GetDefaultSourceAddress();
  }


  for(auto it = path.begin(); it != path.end()-1; ++it)
  {
	  no = INDTONODE.find(*it)->second;
	  rp = no->GetObject<RoutingProtocol>();
	  time = CalculateDelay(*it);
	  Simulator::Schedule(time,&RoutingProtocol::RecvRREP,rp,src,dst,*(it+1));
  }
  return;
}

bool
ControlCenter::IsController(int i)const
{
	return m_controllers.count(i);
}

bool
ControlCenter::IsExistPath(int src, int dst)const
{
	return m_path.count({src,dst});
}


std::vector<int>
ControlCenter::CalculatePath(int src, int dst)
{
	//from <si,sj> to <di,dj>
//	int si,sj,di,dj;
//	si = src/NPERPLANE;
//	sj = src%NPERPLANE;
//	di = dst/NPERPLANE;
//	dj = dst%NPERPLANE;
//	int i = si;
//	int j = sj;
	std::vector<int> path;


//	if(si < di)
//	{
//		if(sj < dj && dj-sj <= sj-dj+NPERPLANE)
//		{
//			/*
//			 * 	<si,sj>
//			 *
//			 * 						<di,dj>
//			 */
//		}
//		if(sj<dj && dj-sj > sj-dj+NPERPLANE)
//		{
//			/*
//			 * 						<di,dj>
//			 *
//			 * 	<si,sj+NPERPLANE>
//			 */
//
//
//		}
//		if(sj == dj)
//		{
//			/*
//			 * 	<si,sj>		<di,dj>
//			 */
//
//		}
//		if(sj > dj && sj-dj <= dj-sj+NPERPLANE)
//		{
//			/*
//			 * 						<di,dj>
//			 *
//			 * 	<si,sj>
//			 */
//
//
//
//		}
//		if(sj > dj && sj-dj > dj-sj+NPERPLANE)
//		{
//			/*
//			 * 	<si,sj>
//			 *
//			 * 				<di,dj+NPERPLANE>
//			 */
//
//		}
//
//	}
//	if(si == di)
//	{
//		if(sj < dj && dj-sj <= sj-dj+NPERPLANE)
//		{
//			/*
//			 * 	<si,sj>
//			 *
//			 * 	<di,dj>
//			 */
//
//		}
//		if(sj < dj && dj-sj > sj-dj+NPERPLANE)
//		{
//			/*
//			 * 	<di,dj>
//			 *
//			 * 	<si,sj+NPERPLANE>
//			 */
//
//		}
//		if(sj == dj)
//		{
//			/*
//			 * <si,sj> == <di,dj>
//			 */
//
//		}
//		if(sj > dj && sj-dj <= dj-sj+NPERPLANE)
//		{
//			/*
//			 * 	<di,dj>
//			 *
//			 * 	<si,sj>
//			 */
//
//		}
//		if(sj > dj && sj-dj > dj-sj+NPERPLANE)
//		{
//			/*
//			 * 	<si,sj>
//			 *
//			 * 	<di,dj+NPERPLANE>
//			 */
//
//		}
//	}
//	if(si > di)
//	{
//		if(sj < dj && dj-sj <= sj-dj+NPERPLANE)
//		{
//			/*
//			 * 						<si,sj>
//			 *
//			 * 	<di,dj>
//			 *
//			 */
//
//
//		}
//		if(sj<dj && dj-sj > sj-dj+NPERPLANE)
//		{
//			/*
//			 * 	<di,dj>
//			 *
//			 * 						<si,sj+NPERPLANE>
//			 */
//
//		}
//		if(sj == dj)
//		{
//			/*
//			 * 	<di,dj>				<si,sj>
//			 */
//
//		}
//		if(sj > dj && sj-dj <= dj-sj+NPERPLANE)
//		{
//			/*
//			 * 	<di,dj>
//			 *
//			 * 						<si,sj>
//			 */
//
//
//		}
//		if(sj > dj && sj-dj > dj-sj+NPERPLANE)
//		{
//			/*
//			 * 						<si,sj>
//			 *
//			 * 	<di,dj+NPERPLANE>
//			 */
//
//		}
//	}

	path.push_back(src);

	/*
	 * 	table:
	 *
	 * 		|path(vector)|node i ... j |
	 * 		|path(vector)|node i ... j |
	 * 		|path(vector)|node i ... j |
	 * 		|path(vector)|node i ... j |
	 *
	 */


	std::vector<std::pair<std::vector<int>,std::vector<double>>> table;
	std::vector<int> flag;

	std::vector<double> cost;
	for(auto it = m_G[src].begin(); it != m_G[src].end(); ++it)
	{
		if(*it == 0)
		{
			cost.push_back(0);
			flag.push_back(1);
			continue;
		}
		if(*it == -1)
		{
			cost.push_back(-1);
			flag.push_back(1);
			continue;
		}
		cost.push_back(m_edges[{src,*it}].delay.GetSeconds());
		flag.push_back(1);
	}
	table.push_back({path,cost});
	flag[src] = 0;

	while(*(table.rbegin()->first.rbegin()) != dst )
	{
		double min_cost = 9999;
		int min_ind;
		std::vector<int> by_path;

		for(auto it = table.begin(); it != table.end(); ++it)
		{
			int ind = 0;
			for(auto iit = it->second.begin(); iit != it->second.end(); ++iit)
			{
				if(flag[ind] == 0) {ind+=1; continue;}
				if(*iit >= 0 && *iit < min_cost)
				{
					min_cost = *iit;
					min_ind = ind;
					by_path = it->first;
				}
				ind+=1;
			}
		}
		by_path.push_back(min_ind);
		flag[min_ind] = 0;
		std::vector<double> cost;
		for(auto it = m_G[min_ind].begin(); it != m_G[min_ind].end(); ++it)
		{
			if(*it == 0)
			{
				cost.push_back(0);
				continue;
			}
			if(*it == -1)
			{
				cost.push_back(-1);
				continue;
			}
			cost.push_back(m_edges[{src,*it}].delay.GetSeconds()+min_cost);
		}
		table.push_back({by_path,cost});
	}

	path = table.rbegin()->first;

	return path;
}

Ipv4Address
ControlCenter::GetGateWay(int cur, int next)
{
	Ptr<Node> c = INDTONODE.find(cur)->second;
	Ptr<Node> n = INDTONODE.find(next)->second;
	Ptr<Ipv4> nip = n->GetObject<Ipv4>();
	for(uint32_t i = 0; i < c->GetNDevices(); ++i)
	{
		Ptr<NetDevice> md = c->GetDevice(i);
		Ptr<Channel> mch = md->GetChannel();
		for(uint32_t j = 0; j< n->GetNDevices(); ++j)
		{
			Ptr<NetDevice> od = n->GetDevice(j);
			Ptr<Channel> och = od->GetChannel();

			if(mch == och)
			{
				return nip->GetAddress(nip->GetInterfaceForDevice(od),0).GetAddress();
			}
		}
	}
	return Ipv4Address();
}

Ptr<NetDevice>
ControlCenter::GetOutputDevice(int cur, int next)
{
	Ptr<Node> c = INDTONODE.find(cur)->second;
	Ptr<Node> n = INDTONODE.find(next)->second;
	for(uint32_t i = 0; i < c->GetNDevices(); ++i)
	{
		Ptr<NetDevice> md = c->GetDevice(i);
		Ptr<Channel> mch = md->GetChannel();
		for(uint32_t j = 0; j< n->GetNDevices(); ++j)
		{
			Ptr<NetDevice> od = n->GetDevice(j);
			Ptr<Channel> och = od->GetChannel();

			if(mch == och)
			{
				return md;
			}
		}
	}
	Ptr<NetDevice> d;
	return d;
}


void
ControlCenter::ChangeEdge(int from, int to, Edge val)
{
  m_edges[{from,to}] = val;
}

void
ControlCenter::SetNum(int num)
{
  m_num = num;
}

void
ControlCenter::InitG()
{
  if(!m_G.empty()) return;
  std::vector<int> temp;
  for(int i = 0; i < m_num; ++i)
    {
      temp.push_back(-1);
    }
  for(int i = 0; i < m_num; ++i)
    {
      m_G.push_back(temp);
    }
  for(int i = 0; i < m_num; ++i)
    {
      m_G[i][i] = 0;
    }
}

void
ControlCenter::ChangeG(int from, int to, int val)
{
  m_G[from][to] = val;
}



}



}
