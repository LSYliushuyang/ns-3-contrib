/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "sdn-helper.h"
#include "ns3/sdn.h"


namespace ns3 {

SDNHelper::SDNHelper():
		Ipv4RoutingHelper()
		{
	m_agentFactory.SetTypeId("ns3::sdn::RoutingProtocol");
		}

SDNHelper*
SDNHelper::Copy(void) const
{
	return new SDNHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
SDNHelper::Create(Ptr<Node> node) const
{
	Ptr<sdn::RoutingProtocol> agent = m_agentFactory.Create<sdn::RoutingProtocol>();
	node->AggregateObject(agent);
	return agent;
}

void
SDNHelper::Set(std::string name, const AttributeValue &value)
{
	m_agentFactory.Set(name,value);
}

/* ... */


}

