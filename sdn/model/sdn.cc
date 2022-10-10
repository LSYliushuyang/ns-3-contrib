/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "sdn.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SDN");

extern std::map<Ptr<Node>,int> NODETOIND;
extern std::map<int,Ptr<Node>> INDTONODE;
extern std::map<Ipv4Address,int> ADDTOIND;

namespace sdn {

NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

class DeferredRouteOutputTag : public Tag
{

public:
  /**
   * \brief Constructor
   * \param o the output interface
   */
  DeferredRouteOutputTag (int32_t o = -1) : Tag (),
                                            m_oif (o)
  {
  }

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ()
  {
    static TypeId tid = TypeId ("ns3::sdn::DeferredRouteOutputTag")
      .SetParent<Tag> ()
      .SetGroupName ("SDN")
      .AddConstructor<DeferredRouteOutputTag> ()
    ;
    return tid;
  }

  TypeId  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  /**
   * \brief Get the output interface
   * \return the output interface
   */
  int32_t GetInterface () const
  {
    return m_oif;
  }

  /**
   * \brief Set the output interface
   * \param oif the output interface
   */
  void SetInterface (int32_t oif)
  {
    m_oif = oif;
  }

  uint32_t GetSerializedSize () const
  {
    return sizeof(int32_t);
  }

  void  Serialize (TagBuffer i) const
  {
    i.WriteU32 (m_oif);
  }

  void  Deserialize (TagBuffer i)
  {
    m_oif = i.ReadU32 ();
  }

  void  Print (std::ostream &os) const
  {
    os << "DeferredRouteOutputTag: output interface = " << m_oif;
  }

private:
  /// Positive if output device is fixed in RouteOutput
  int32_t m_oif;
};

NS_OBJECT_ENSURE_REGISTERED (DeferredRouteOutputTag);



const uint32_t RoutingProtocol::SDN_PORT = 321;

ControlCenter RoutingProtocol::NETCENTER = ControlCenter();

//FlowTable RoutingProtocol::GLOBAL_FLOWTABLE = FlowTable();
//
//FlowTable RoutingProtocol::LOCAL_FLOWTABLE = FlowTable();
//
//LookupTable RoutingProtocol::LOOKUP_TABLE = LookupTable();

TypeId
RoutingProtocol::GetTypeId(void)
{
  static TypeId tid = TypeId ("ns3::sdn::RouringProtocol")
      .SetParent<Ipv4RoutingProtocol>()
      .SetGroupName ("SDN")
      .AddConstructor<RoutingProtocol>()
      ;
  return tid;
}

Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header,
                              Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  if (!p)
    {
      NS_LOG_DEBUG ("Packet is == 0" );
      return LoopbackRoute (header, oif);
    }
  if (m_socketAddresses.empty ())
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
      NS_LOG_LOGIC ("No sdn interfaces");
      Ptr<Ipv4Route> route;
      return route;
    }

  sockerr = Socket::ERROR_NOTERROR;

  Ipv4Address dst = header.GetDestination();
  Ipv4Address src = header.GetSource();

  if(m_flowtable.IsExist(src, dst))
  {
	  return m_flowtable.Get(src,dst);
  }
  else
  {
	  //Send RREQ
//	  int src_ind = ADDTOIND.find(src)->second;
//	  int dst_ind = ADDTOIND.find(dst)->second;
	  int this_ind = NODETOIND.find(this->GetObject<Node>())->second;
	  Time time = NETCENTER.CalculateDelay(this_ind);
	  Simulator::Schedule(time, &ControlCenter::RecvRREQ,&NETCENTER,this_ind,src,dst);
	  uint32_t iif = (oif ? m_ipv4->GetInterfaceForDevice (oif) : -1);
	  DeferredRouteOutputTag tag (iif);
	  NS_LOG_DEBUG ("Can not find flow-table-item match the transmission-flow.");
	  if (!p->PeekPacketTag (tag))
	    {
	      p->AddPacketTag (tag);
	    }
	  return LoopbackRoute (header, oif);
  }
}

bool
RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header,
                             Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                             MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb)
{
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No sdsn interfaces");
      return false;
    }
  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address src = header.GetSource ();

  if (idev == m_lo)
    {
      DeferredRouteOutputTag tag;
      if (p->PeekPacketTag (tag))
        {
          DeferredRouteOutput (p, header, ucb, ecb);
          return true;
        }
    }

  if (IsMyOwnAddress (src))
    {
      return true;
    }

  if (m_ipv4->IsDestinationAddress (dst, iif))
    {
      if (lcb.IsNull () == false)
        {
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          lcb (p, header, iif);
        }
      else
        {
          NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << src);
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
      return true;
    }

  if (m_ipv4->IsForwarding (iif) == false)
    {
      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      ecb (p, header, Socket::ERROR_NOROUTETOHOST);
      return true;
    }

  // Forwarding
  return Forwarding (p, header, ucb, ecb);
}

void
RoutingProtocol::NotifyInterfaceUp (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (l3->GetNAddresses (i) > 1)
    {
      NS_LOG_WARN ("SDSN does not work with more then one address per each interface.");
    }
  Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
  if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
    {
      return;
    }

  // Create a socket to listen only on this interface
  Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                             UdpSocketFactory::GetTypeId ());
  NS_ASSERT (socket != 0);
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvControlPacket, this));
  socket->BindToNetDevice (l3->GetNetDevice (i));
  socket->Bind (InetSocketAddress (iface.GetLocal (), SDN_PORT));
  socket->SetAllowBroadcast (false);
  socket->SetIpRecvTtl (true);
  m_socketAddresses.insert (std::make_pair (socket, iface));
}

void
RoutingProtocol::NotifyInterfaceDown (uint32_t i)
{
  NS_LOG_FUNCTION (this << m_ipv4->GetAddress (i, 0).GetLocal ());

  // Disable layer 2 link state monitoring (if possible)
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  Ptr<NetDevice> dev = l3->GetNetDevice (i);

  // Close socket
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (m_ipv4->GetAddress (i, 0));
  NS_ASSERT (socket);
  socket->Close ();
  m_socketAddresses.erase (socket);

  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No sdn interfaces");

      return;
    }
}

void
RoutingProtocol::NotifyAddAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << " interface " << i << " address " << address);
  Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
  if (!l3->IsUp (i))
    {
      return;
    }
  if (l3->GetNAddresses (i) == 1)
    {
      Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
      Ptr<Socket> socket = FindSocketWithInterfaceAddress (iface);
      if (!socket)
        {
          if (iface.GetLocal () == Ipv4Address ("127.0.0.1"))
            {
              return;
            }
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvControlPacket,this));
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetLocal (), SDN_PORT));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));
        }
    }
  else
    {
      NS_LOG_LOGIC ("SDN does not work with more then one address per each interface. Ignore added address");
    }
}

void
RoutingProtocol::NotifyRemoveAddress (uint32_t i, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this);
  Ptr<Socket> socket = FindSocketWithInterfaceAddress (address);
  if (socket)
    {
      //m_routingtable.DeleteAllRoutesFromInterface (address);
      socket->Close ();
      m_socketAddresses.erase (socket);

      Ptr<Ipv4L3Protocol> l3 = m_ipv4->GetObject<Ipv4L3Protocol> ();
      if (l3->GetNAddresses (i))
        {
          Ipv4InterfaceAddress iface = l3->GetAddress (i, 0);
          // Create a socket to listen only on this interface
          Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
                                                     UdpSocketFactory::GetTypeId ());
          NS_ASSERT (socket != 0);
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvControlPacket, this));
          // Bind to any IP address so that broadcasts can be received
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetLocal (), SDN_PORT));
          socket->SetAllowBroadcast (false);
          socket->SetIpRecvTtl (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));
        }
      if (m_socketAddresses.empty ())
        {
          NS_LOG_LOGIC ("No sdsn interfaces");
          return;
        }
    }
  else
    {
      NS_LOG_LOGIC ("Remove address not participating in SDSN operation");
    }
}

void
RoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_ASSERT (ipv4 != 0);
  NS_ASSERT (m_ipv4 == 0);

  m_ipv4 = ipv4;

  // Create lo route. It is asserted that the only one interface up for now is loopback
  NS_ASSERT (m_ipv4->GetNInterfaces () == 1 && m_ipv4->GetAddress (0, 0).GetLocal () == Ipv4Address ("127.0.0.1"));
  m_lo = m_ipv4->GetNetDevice (0);
  NS_ASSERT (m_lo != 0);

  Ipv4Route rt;
  rt.SetOutputDevice(m_lo);
  rt.SetDestination(Ipv4Address::GetLoopback());
  rt.SetGateway(Ipv4Address::GetLoopback());
  for(auto it = m_socketAddresses.begin(); it != m_socketAddresses.end(); ++it)
  {
	  rt.SetSource(it->second.GetAddress());
	  m_flowtable.Add(rt.GetSource(), rt.GetDestination(), rt);
  }
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
}

RoutingProtocol::RoutingProtocol()
  :
    m_queue (100, Seconds(30)),
    m_interval (Seconds(1)),
    m_seqNo (0)
{
  m_htimer.SetFunction(&RoutingProtocol::HelloTimerExpire, this);
  uint32_t startTime = rand()%100;
  m_htimer.Schedule (MilliSeconds(startTime));
}

void
RoutingProtocol::RecvControlPacket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Address sourceAddress;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddress);
  InetSocketAddress inetSourceAddr = InetSocketAddress::ConvertFrom (sourceAddress);
  Ipv4Address sender = inetSourceAddr.GetIpv4 ();
  Ipv4Address receiver;

  if (m_socketAddresses.find (socket) != m_socketAddresses.end ())
    {
      receiver = m_socketAddresses[socket].GetLocal ();
    }

  NS_LOG_DEBUG ("SDSN node " << this << " received a SDSN packet from " << sender << " to " << receiver);

//  TypeHeader tHeader (SDNTYPE_RREQ);
//  packet->RemoveHeader (tHeader);
//  if (!tHeader.IsValid ())
//    {
//      NS_LOG_DEBUG ("SDN message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
//      return; // drop
//    }
//  switch (tHeader.Get ())
//    {
//    case SDNTYPE_RREQ:
//      {
////        RecvRequest (packet, receiver, sender);
//        break;
//      }
//    case SDNTYPE_RREP:
//      {
////        RecvReply (packet, receiver, sender);
//        break;
//      }
//    case SDNTYPE_HELLO:
//      {
////        RecvHello (packet, receiver, sender);
//        break;
//      }
//    case SDNTYPE_CONFIG:;
//    }
}

void
RoutingProtocol::HelloTimerExpire ()
{
//  SendHello();
  m_htimer.Cancel();
  m_htimer.Schedule(std::max (Time (Seconds (0)), m_interval));
}

Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{
  NS_LOG_FUNCTION (this << hdr);
  NS_ASSERT (m_lo != 0);
  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination (hdr.GetDestination ());
  std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin ();
  if (oif)
    {
      // Iterate to find an address on the oif device
      for (j = m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
        {
          Ipv4Address addr = j->second.GetLocal ();
          int32_t interface = m_ipv4->GetInterfaceForAddress (addr);
          if (oif == m_ipv4->GetNetDevice (static_cast<uint32_t> (interface)))
            {
              rt->SetSource (addr);
              break;
            }
        }
    }
  else
    {
      rt->SetSource (j->second.GetLocal ());
    }
  NS_ASSERT_MSG (rt->GetSource () != Ipv4Address (), "Valid SDSN source address not found");
  rt->SetGateway (Ipv4Address ("127.0.0.1"));
  rt->SetOutputDevice (m_lo);
  return rt;
}

void
RoutingProtocol::DeferredRouteOutput (Ptr<const Packet> p, const Ipv4Header & header, UnicastForwardCallback ucb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p << header);
  NS_ASSERT (p != 0 && p != Ptr<Packet> ());

  QueueEntry newEntry (p, header, ucb, ecb);
  bool result = m_queue.Enqueue (newEntry);
  if (result)
    {
      NS_LOG_LOGIC ("Add packet " << p->GetUid () << " to queue. Protocol " << (uint16_t) header.GetProtocol ());
//      Ipv4Route rt;
//      Ipv4Address src = header.GetSource();
//      Ipv4Address dst = header.GetDestination();
//      bool result = LOCAL_FLOWTABLE.IsExist(src, dst);
////          m_routingtable.LookupRoute (header.GetDestination (), rt);
//      if (!result)
//        {
//          NS_LOG_LOGIC ("Send new RREQ for outbound packet to " << header.GetDestination ());
//
//          ControlPacketHeader rrh;
//          rrh.SetDst(header.GetDestination());
//          rrh.SetOrigin(m_outSocket.second.GetLocal());
//
//          Ptr<Packet> packet = Create<Packet> ();
//          SocketIpTtlTag tag;
//          tag.SetTtl (30);
//          packet->AddPacketTag (tag);
//          packet->AddHeader (rrh);
//          TypeHeader tHeader (SDNTYPE_RREQ);
//          packet->AddHeader (tHeader);
//          Simulator::Schedule (Seconds(0), &RoutingProtocol::SendTo, this, m_outSocket.first, packet, m_conIp);
//        }
    }
}

void
RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
{
  socket->SendTo (packet, 0, InetSocketAddress (destination, SDN_PORT));
}

bool
RoutingProtocol::IsMyOwnAddress (Ipv4Address src)
{
  NS_LOG_FUNCTION (this << src);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ipv4InterfaceAddress iface = j->second;
      if (src == iface.GetLocal ())
        {
          return true;
        }
    }
  return false;
}

bool
RoutingProtocol::Forwarding (Ptr<const Packet> p, const Ipv4Header & header,
                             UnicastForwardCallback ucb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this);
//  Ipv4Address dst = header.GetDestination ();
//  Ipv4Address origin = header.GetSource ();
  Ipv4Route toDst;
//  if (LOCAL_FLOWTABLE.IsExist(origin,dst))//m_routingtable.LookupRoute (dst, toDst))
//    {
//      FlowTableItem fti = LOCAL_FLOWTABLE.Find(origin,dst);
//      if(IsMyOwnAddress(origin))
//        {
//          toDst.SetSource(origin);
//          toDst.SetDestination(dst);
//          toDst.SetGateway(fti.GetAddress(0));
//          toDst.SetOutputDevice(fti.GetNetDevice(0));
//          Ptr<Ipv4Route> route = &toDst;
//          NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());
//          ucb (route, p, header);
//          return true;
//        }
//      int hop = fti.GetHop();
//      for(int i = 0; i < hop - 1; ++i)
//        {
//          if(IsMyOwnAddress(fti.GetAddress(i)))
//            {
//              toDst.SetSource(origin);
//              toDst.SetDestination(dst);
//              toDst.SetGateway(fti.GetAddress(i+1));
//              toDst.SetOutputDevice(fti.GetNetDevice(i+1));
//              Ptr<Ipv4Route> route = &toDst;
//              NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());
//              ucb (route, p, header);
//              return true;
//            }
//        }
//    }
//  NS_LOG_LOGIC ("route not found to " << dst << ". Send RERR message.");
  NS_LOG_DEBUG ("Drop packet " << p->GetUid () << " because no route to forward it.");
  return false;
}

//void
//RoutingProtocol::RecvRequest (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src)
//{
//  NS_LOG_FUNCTION (this);
//  ControlPacketHeader rreqHeader;
//  p->RemoveHeader (rreqHeader);
//
//  Ipv4Address origin = rreqHeader.GetOrigin ();
//
//  Ipv4Address dest = rreqHeader.GetDst();
//
//  if(!GLOBAL_FLOWTABLE.IsExist(origin,dest))
//    {
//
//	  //generate flow path from origin to dest
//
//	  /*
//	   *
//	   *
//	   *
//	   *
//	   *
//	   */
//
//
//    }
//  Ipv4Route toOrigin;
//  NS_LOG_DEBUG ("Send reply since I am the destination");
//
//  if(!LOCAL_FLOWTABLE.IsExist(receiver,origin))
//    {
//      NS_LOG_DEBUG("Can not find flow-table-item from controller to switch.");
//      return;
//    }
//
//  FlowTableItem fti = LOCAL_FLOWTABLE.Find(receiver,origin);
//  toOrigin.SetDestination(origin);
//  toOrigin.SetGateway(fti.GetAddress(0));
//  toOrigin.SetOutputDevice(fti.GetNetDevice(0));
//  toOrigin.SetSource(receiver);
//  SendReply (rreqHeader, toOrigin);
//  return;
//}

//void
//RoutingProtocol::SendReply (ControlPacketHeader const & rreqHeader, Ipv4Route const & toOrigin)
//{
//  NS_LOG_FUNCTION (this << toOrigin.GetDestination ());
//
//  ControlPacketHeader rrepHeader;
//  rrepHeader.SetDst(rreqHeader.GetDst());
//  rrepHeader.SetOrigin(toOrigin.GetDestination ());
//
//
//  Ptr<Packet> packet = Create<Packet> ();
//  SocketIpTtlTag tag;
//  tag.SetTtl (30);
//  packet->AddPacketTag (tag);
//  packet->AddHeader (rrepHeader);
//  TypeHeader tHeader (SDNTYPE_RREP);
//  packet->AddHeader (tHeader);
//
//  Ptr<Socket> socket = FindSocketWithInterfaceAddress (Ipv4InterfaceAddress(toOrigin.GetSource(),"255.255.255.0"));
//  NS_ASSERT (socket);
//  socket->SendTo (packet, 0, InetSocketAddress (toOrigin.GetDestination(), SDN_PORT));
//}

Ptr<Socket>
RoutingProtocol::FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const
{
  NS_LOG_FUNCTION (this << addr);
  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j =
         m_socketAddresses.begin (); j != m_socketAddresses.end (); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      if (iface == addr)
        {
          return socket;
        }
    }
  Ptr<Socket> socket;
  return socket;
}

//void
//RoutingProtocol::RecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
//{
//  NS_LOG_FUNCTION (this << " src " << sender);
//  ControlPacketHeader rrepHeader;
//  p->RemoveHeader (rrepHeader);
//  Ipv4Address dst = rrepHeader.GetDst ();
//  Ipv4Address src = rrepHeader.GetOrigin();
//  NS_LOG_LOGIC ("RREP destination " << dst << " RREP origin " << rrepHeader.GetOrigin ());
//  NS_LOG_LOGIC ("add new flow path");
//
//  //update LOCAL_FLOWTABLE from GLOBAL_FLOWTABLE
//  /*
//   *
//   *
//   *
//   *
//   */
//
//
//
//
//  if (IsMyOwnAddress (src))
//    {
//      Ipv4Route toDst;
//      if(!LOCAL_FLOWTABLE.IsExist(src,dst))
//        {
//          NS_LOG_DEBUG("Can not update LOCAL_FLOWTABLE from RREP.");
//          return;
//        }
//      FlowTableItem fti = LOCAL_FLOWTABLE.Find(src,dst);
//      toDst.SetDestination(dst);
//      toDst.SetGateway(fti.GetAddress(0));
//      toDst.SetOutputDevice(fti.GetNetDevice(0));
//      toDst.SetSource(src);
//      SendPacketFromQueue (dst, &toDst);
//      return;
//    }
//}

//void
//RoutingProtocol::RecvHello (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
//{
//}
//
//void
//RoutingProtocol::RecvConfig (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
//{
//}

void
RoutingProtocol::SendPacketFromQueue (Ipv4Address src, Ipv4Address dst, Ptr<Ipv4Route> route)
{
  NS_LOG_FUNCTION (this);
  QueueEntry queueEntry;
  while (m_queue.Dequeue (src, dst, queueEntry))
    {
      DeferredRouteOutputTag tag;
      Ptr<Packet> p = ConstCast<Packet> (queueEntry.GetPacket ());
      if (p->RemovePacketTag (tag)
          && tag.GetInterface () != -1
          && tag.GetInterface () != m_ipv4->GetInterfaceForDevice (route->GetOutputDevice ()))
        {
          NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
          return;
        }
      UnicastForwardCallback ucb = queueEntry.GetUnicastForwardCallback ();
      Ipv4Header header = queueEntry.GetIpv4Header ();
      header.SetSource (route->GetSource ());
      header.SetTtl (header.GetTtl () + 1); // compensate extra TTL decrement by fake loopback routing
      ucb (route, p, header);
    }
}

void
RoutingProtocol::RecvRREP(Ipv4Address src, Ipv4Address dst, int next)
{
	int this_no = NODETOIND.find(this->GetObject<Node>())->second;
	Ptr<Ipv4Route> route = Create<Ipv4Route>();
	route->SetSource(src);
	route->SetDestination(dst);
	route->SetGateway(NETCENTER.GetGateWay(this_no,next));
	route->SetOutputDevice(NETCENTER.GetOutputDevice(this_no,next));
	m_flowtable.Delete(src,dst);
	m_flowtable.Add(src,dst,route);
	SendPacketFromQueue(src,dst,route);
}

//void
//RoutingProtocol::SendHello ()
//{
//  NS_LOG_FUNCTION (this);
//
//  if(m_type == CONTROLLER)
//    {
//      /*
//       *
//       *
//       *
//       *
//       *
//       *
//       *
//       *
//       */
//
//    }
//  ControlPacketHeader helloHeader;
//  helloHeader.SetDst(m_conIp);
//  helloHeader.SetOrigin(m_outSocket.second.GetLocal());
//  Ptr<Packet> packet = Create<Packet> ();
//
//  SocketIpTtlTag tag;
//  tag.SetTtl (30);
//  packet->AddPacketTag (tag);
//  packet->AddHeader (helloHeader);
//  TypeHeader tHeader (SDNTYPE_HELLO);
//  packet->AddHeader(tHeader);
//  /*
//   *
//   *
//   *
//   *
//   *
//   *
//   *
//   *
//   *
//   *
//   *
//   */
//  Simulator::Schedule(Seconds(0),&RoutingProtocol::SendTo, this, m_outSocket.first, packet, m_conIp);
//}






















}

/* ... */


}

