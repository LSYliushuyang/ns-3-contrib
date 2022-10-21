/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SDN_H
#define SDN_H

#include "ns3/ipv4-routing-protocol.h"
#include "sdn-flow-table.h"
#include "sdn-rqueue.h"
#include "ns3/timer.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/udp-socket-factory.h"
#include "sdn-netview.h"
#include "ns3/node.h"




namespace ns3 {



namespace sdn {

class ControlCenter;

class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  static const uint32_t SDN_PORT;
  static ControlCenter NETCENTER;

  static TypeId GetTypeId(void);

public:
  virtual Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  virtual bool RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                              UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                              LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;

public:

  RoutingProtocol();

  void RecvControlPacket (Ptr<Socket> socket);

  void RecvRREP(Ipv4Address,Ipv4Address,int);

  void HelloTimerExpire ();

  void SetHelloInterval(Time time){m_interval = time;}

  Ipv4Address GetDefaultSourceAddress();

private:

  Ptr<Ipv4Route> LoopbackRoute (const Ipv4Header & header, Ptr<NetDevice> oif) const;
  void DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb);
  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);
  bool IsMyOwnAddress (Ipv4Address src);
  bool Forwarding (Ptr<const Packet> p, const Ipv4Header & header,
                   UnicastForwardCallback ucb, ErrorCallback ecb);

//  void RecvRequest (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src);

//  void SendReply (ControlPacketHeader const & conPacHeader, Ipv4Route const & toOrigin);

  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const;

//  void RecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

//  void RecvHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

//  void RecvConfig(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

  void SendPacketFromQueue (Ipv4Address src, Ipv4Address dst, Ptr<Ipv4Route> route);

  void SendHello ();


private:
  Ptr<NetDevice> m_lo;
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;
  Ptr<Ipv4> m_ipv4;
  RequestQueue m_queue;


  Timer m_htimer;
  Time m_interval;

  uint32_t m_seqNo;

  FlowTable m_flowtable;





};




}

/* ... */

}

#endif /* SDN_H */

