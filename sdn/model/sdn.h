/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SDN_H
#define SDN_H

#include "ns3/ipv4-routing-protocol.h"
#include "sdn-packet.h"
#include "sdn-flowtable.h"
#include "sdn-rqueue.h"
#include "ns3/timer.h"
#include "sdn-id-lookup-table.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/udp-socket-factory.h"

namespace ns3 {

namespace sdn {

enum NodeType {SWITCH, CONTROLLER};

class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  static const uint32_t SDN_PORT;
  static FlowTable GLOBAL_FLOWTABLE;
  static FlowTable LOCAL_FLOWTABLE;
  static LookupTable LOOKUP_TABLE;

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

  NodeType GetNodeType(){return m_type;}

  void SetNodeType(NodeType nt){m_type = nt;}

  RoutingProtocol();
//  :m_queue(100,Seconds(30)),m_type(SWITCH),m_interval(Seconds(1)),m_controller_seqNo(0),m_seqNo(0)
//  {
//    m_htimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
//    uint32_t startTime = rand()%100;
//    m_htimer.Schedule (MilliSeconds (startTime));


//    m_htimer.Schedule (Seconds(0));
//  };

  void RecvControlPacket (Ptr<Socket> socket);

  void SetOutSocket(Ipv4InterfaceAddress add);

  void HelloTimerExpire ();

  void SetHelloInterval(Time time){m_interval = time;}

private:

  Ptr<Ipv4Route> LoopbackRoute (const Ipv4Header & header, Ptr<NetDevice> oif) const;
  void DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb);
  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);
  bool IsMyOwnAddress (Ipv4Address src);
  bool Forwarding (Ptr<const Packet> p, const Ipv4Header & header,
                   UnicastForwardCallback ucb, ErrorCallback ecb);

  void RecvRequest (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src);

  void SendReply (ControlPacketHeader const & conPacHeader, Ipv4Route const & toOrigin);

  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const;

  void RecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

  void RecvHello(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

  void RecvConfig(Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

  void SendPacketFromQueue (Ipv4Address dst, Ptr<Ipv4Route> route);

  void SendHello ();


private:
  Ptr<NetDevice> m_lo;
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;
  Ptr<Ipv4> m_ipv4;
  RequestQueue m_queue;

  NodeType m_type;

  std::pair<Ptr<Socket>,Ipv4InterfaceAddress> m_outSocket;

  Timer m_htimer;
  Time m_interval;

  uint32_t m_seqNo;

  uint32_t m_controller_seqNo;

  std::vector<uint32_t> m_switch_seqNos;

  Ipv4Address m_conIp;

//  std::pair<int,int> a;




};




}

/* ... */

}

#endif /* SDN_H */

