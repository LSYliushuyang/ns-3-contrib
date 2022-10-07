/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "sdsn-helper.h"

namespace ns3 {

SdsnHelper::SdsnHelper()
    : Ipv4RoutingHelper()
      {
  m_agentFactory.SetTypeId("ns3::sdsn::RoutingProtocol");
      }

SdsnHelper*
SdsnHelper::Copy(void) const
{
  return new SdsnHelper (*this);
}

Ptr<Ipv4RoutingProtocol>
SdsnHelper::Create(Ptr<Node> no) const
{
  Ptr<sdsn::RoutingProtocol> agent = m_agentFactory.Create<sdsn::RoutingProtocol>();
  no->AggregateObject(agent);
  return agent;
}


/* ... */


}

