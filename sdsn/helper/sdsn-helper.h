/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SDSN_HELPER_H
#define SDSN_HELPER_H

#include "ns3/sdsn.h"
#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

class SdsnHelper : public Ipv4RoutingHelper
{
public:
  SdsnHelper* Copy()const;

  Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const;

  SdsnHelper();

private:
  ObjectFactory m_agentFactory;

};

/* ... */

}

#endif /* SDSN_HELPER_H */

