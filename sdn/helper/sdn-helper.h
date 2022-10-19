/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SDN_HELPER_H
#define SDN_HELPER_H

#include "ns3/sdn.h"
#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

class SDNHelper : public Ipv4RoutingHelper
{
public:
	SDNHelper();

	SDNHelper* Copy(void) const;

	virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const;

	void Set(std::string name, const AttributeValue &value);

private:
	ObjectFactory m_agentFactory;
};

/* ... */

}

#endif /* SDN_HELPER_H */

