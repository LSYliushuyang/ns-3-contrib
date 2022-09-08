/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "sdsn.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SDSN");



namespace sdsn
{

NS_OBJECT_ENSURE_REGISTERED (RoutingProtocol);

const uint32_t RoutingProtocol::SDSN_PORT = 654;

NetView::NetView()
{

}

NetView RoutingProtocol::LogCen = NetView();

std::ostream &
operator<< (std::ostream & os, RRHeader const & h)
{
  h.Print (os);
  return os;
}

std::ostream & operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}

void
NetView::Install(NodeContainer c)
{
  uint32_t node_num = c.GetN();
  for(uint32_t i = 0; i < node_num; i++)
    {
      uint32_t device_num = c.Get(i)->GetNDevices();
      std::list<std::pair<Ptr<Node>,NetViewEdge>> temp;
      temp.push_back(std::make_pair(c.Get(i),NetViewEdge()));
      for(uint32_t j = 0; j < device_num; j++)
        {
          Ptr<NetDevice> dev = c.Get(i)->GetDevice(j);
          Ptr<Channel> cha = dev->GetChannel();
          if(cha)
            {
              if(cha->GetNDevices() == 2)
                {
                  Ptr<NetDevice> odev = cha->GetDevice(0) == dev ? cha->GetDevice(1) : cha->GetDevice(0);
                  temp.push_back(std::make_pair(odev->GetNode(),NetViewEdge(dev,odev,cha)));
                }
            }
        }
      m_view.insert(std::make_pair(c.Get(i),temp));
    }
}

std::pair<Time,Time>
NetView::GetDelay(Ptr<Node> node1, Ptr<Node> node2)
{
  for(Adjlist::iterator it = m_view.begin(); it != m_view.end(); it++)
    {
      if(it->second.begin()->first == node1)
        {
          for(LList::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
            {
              if(iter->first == node2)
                {
                  return std::make_pair(Simulator::Now(),iter->second.GetDelay());
                }
            }
        }
    }
  return std::make_pair(Simulator::Now(),Seconds(99999));
}

std::pair<Time,double>
NetView::GetLoad(Ptr<Node> node1, Ptr<Node> node2)
{
  for(Adjlist::iterator it = m_view.begin(); it != m_view.end(); it++)
    {
      if(it->second.begin()->first == node1)
        {
          for(LList::iterator iter = it->second.begin(); iter != it->second.end(); iter++)
            {
              if(iter->first == node2)
                {
                  return std::make_pair(Simulator::Now(),iter->second.GetLoad());
                }
            }
        }
    }
  return std::make_pair(Simulator::Now(),1);
}

bool
RoutingTable::LookupRoute (Ipv4Address id, Ipv4Route & rt)
{
  if(m_table.empty())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found; m_table is empty");
      return false;
    }
  std::map<Ipv4Address, Ipv4Route>::const_iterator i =
    m_table.find (id);
  if (i == m_table.end ())
    {
      NS_LOG_LOGIC ("Route to " << id << " not found");
      return false;
    }
  rt = i->second;
  NS_LOG_LOGIC ("Route to " << id << " found");
  return true;
}

TypeId
RoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::sdsn::RoutingProtocol")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("sdsn")
    .AddConstructor<RoutingProtocol> ()
    ;
    return tid;
}

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

Ptr<Ipv4Route>
RoutingProtocol::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr)
{
  if (!p)
    {
      NS_LOG_DEBUG ("Packet is == 0");
      return LoopbackRoute (header, oif); // later
    }
  if (m_socketAddresses.empty ())
    {
      sockerr = Socket::ERROR_NOROUTETOHOST;
      NS_LOG_LOGIC ("No aodv interfaces");
      Ptr<Ipv4Route> route;
      return route;
    }
  sockerr = Socket::ERROR_NOTERROR;
  Ptr<Ipv4Route> route = Create<Ipv4Route>();
  Ipv4Address dst = header.GetDestination ();
  if (m_routingtable.LookupRoute (dst, *route))
    {
      NS_ASSERT (route != 0);
      NS_LOG_DEBUG ("Exist route to " << route->GetDestination () << " from interface " << route->GetSource ());
      if (oif != 0 && route->GetOutputDevice () != oif)
        {
          NS_LOG_DEBUG ("Output device doesn't match. Dropped.");
          sockerr = Socket::ERROR_NOROUTETOHOST;
          return Ptr<Ipv4Route> ();
        }
      return route;
    }

  // Valid route not found, in this case we return loopback.
  // Actual route request will be deferred until packet will be fully formed,
  // routed to loopback, received from loopback and passed to RouteInput (see below)
  uint32_t iif = (oif ? m_ipv4->GetInterfaceForDevice (oif) : -1);
  DeferredRouteOutputTag tag (iif);
  NS_LOG_DEBUG ("Valid Route not found");
  if (!p->PeekPacketTag (tag))
    {
      p->AddPacketTag (tag);
    }
  return LoopbackRoute (header, oif);
}

Ptr<Ipv4Route>
RoutingProtocol::LoopbackRoute (const Ipv4Header & hdr, Ptr<NetDevice> oif) const
{
  NS_LOG_FUNCTION (this << hdr);
  NS_ASSERT (m_lo != 0);
  Ptr<Ipv4Route> rt = Create<Ipv4Route> ();
  rt->SetDestination (hdr.GetDestination ());
  //
  // Source address selection here is tricky.  The loopback route is
  // returned when AODV does not have a route; this causes the packet
  // to be looped back and handled (cached) in RouteInput() method
  // while a route is found. However, connection-oriented protocols
  // like TCP need to create an endpoint four-tuple (src, src port,
  // dst, dst port) and create a pseudo-header for checksumming.  So,
  // AODV needs to guess correctly what the eventual source address
  // will be.
  //
  // For single interface, single address nodes, this is not a problem.
  // When there are possibly multiple outgoing interfaces, the policy
  // implemented here is to pick the first available AODV interface.
  // If RouteOutput() caller specified an outgoing interface, that
  // further constrains the selection of source address
  //
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
RoutingProtocol::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
{
  socket->SendTo (packet, 0, InetSocketAddress (destination, SDSN_PORT));

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
      Ipv4Route rt;
      bool result = m_routingtable.LookupRoute (header.GetDestination (), rt);
      if (!result)
        {
          NS_LOG_LOGIC ("Send new RREQ for outbound packet to " << header.GetDestination ());

          RRHeader rrh;
          rrh.SetDst(header.GetDestination());
          rrh.SetOriginAdd(m_outSocket.second.GetLocal());

          Ptr<Packet> packet = Create<Packet> ();
          SocketIpTtlTag tag;
          tag.SetTtl (30);
          packet->AddPacketTag (tag);
          packet->AddHeader (rrh);
          TypeHeader tHeader (SDSNTYPE_RREQ);
          packet->AddHeader (tHeader);
          Simulator::Schedule (Simulator::Now(), &RoutingProtocol::SendTo, this, m_outSocket.first, packet, m_conIP);
        }
    }

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
  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();
  Ipv4Route toDst;
  if (m_routingtable.LookupRoute (dst, toDst))
    {

          Ptr<Ipv4Route> route = &toDst;
          NS_LOG_LOGIC (route->GetSource () << " forwarding to " << dst << " from " << origin << " packet " << p->GetUid ());

          ucb (route, p, header);
          return true;
    }
  NS_LOG_LOGIC ("route not found to " << dst << ". Send RERR message.");
  NS_LOG_DEBUG ("Drop packet " << p->GetUid () << " because no route to forward it.");
  return false;

}

bool
RoutingProtocol::RouteInput  (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                            UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                            LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (this << p->GetUid () << header.GetDestination () << idev->GetAddress ());
  if (m_socketAddresses.empty ())
    {
      NS_LOG_LOGIC ("No sdsn interfaces");
      return false;
    }
  NS_ASSERT (m_ipv4 != 0);
  NS_ASSERT (p != 0);
  // Check if input device supports IP
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  int32_t iif = m_ipv4->GetInterfaceForDevice (idev);

  Ipv4Address dst = header.GetDestination ();
  Ipv4Address origin = header.GetSource ();

  // Deferred route request
  if (idev == m_lo)
    {
      DeferredRouteOutputTag tag;
      if (p->PeekPacketTag (tag))
        {
          DeferredRouteOutput (p, header, ucb, ecb);
          return true;
        }
    }

  // Duplicate of own packet
  if (IsMyOwnAddress (origin))
    {
      return true;
    }

  // AODV is not a multicast routing protocol
  if (dst.IsMulticast ())
    {
      return false;
    }

  // Unicast local delivery
  if (m_ipv4->IsDestinationAddress (dst, iif))
    {
      if (lcb.IsNull () == false)
        {
          NS_LOG_LOGIC ("Unicast local delivery to " << dst);
          lcb (p, header, iif);
        }
      else
        {
          NS_LOG_ERROR ("Unable to deliver packet locally due to null callback " << p->GetUid () << " from " << origin);
          ecb (p, header, Socket::ERROR_NOROUTETOHOST);
        }
      return true;
    }

  // Check if input device supports IP forwarding
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
RoutingProtocol::SendReply (RRHeader const & rreqHeader, Ipv4Route const & toOrigin)
{
  NS_LOG_FUNCTION (this << toOrigin.GetDestination ());

  RRHeader rrepHeader;
  rrepHeader.SetDst(rreqHeader.GetDst());
  rrepHeader.SetOriginAdd(toOrigin.GetDestination ());


  Ptr<Packet> packet = Create<Packet> ();
  SocketIpTtlTag tag;
  tag.SetTtl (30);
  packet->AddPacketTag (tag);
  packet->AddHeader (rrepHeader);
  TypeHeader tHeader (SDSNTYPE_RREP);
  packet->AddHeader (tHeader);

  Ptr<Socket> socket = FindSocketWithInterfaceAddress (Ipv4InterfaceAddress(toOrigin.GetSource(),"255.255.255.0"));
  NS_ASSERT (socket);
  socket->SendTo (packet, 0, InetSocketAddress (toOrigin.GetDestination(), SDSN_PORT));
}


void
RoutingProtocol::RecvRequest (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address src)
{
  NS_LOG_FUNCTION (this);
  RRHeader rreqHeader;
  p->RemoveHeader (rreqHeader);

  Ipv4Address origin = rreqHeader.GetOrigin ();


  //  A node generates a RREP if either:
  //  (i)  it is itself the destination,
  Ipv4Route toOrigin;
  if (IsMyOwnAddress (rreqHeader.GetDst ()))
    {
      m_routingtable.LookupRoute (origin, toOrigin);
      NS_LOG_DEBUG ("Send reply since I am the destination");
      SendReply (rreqHeader, toOrigin);
      return;
    }
}

void
RoutingProtocol::SendPacketFromQueue (Ipv4Address dst, Ptr<Ipv4Route> route)
{
  NS_LOG_FUNCTION (this);
  QueueEntry queueEntry;
  while (m_queue.Dequeue (dst, queueEntry))
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
RoutingProtocol::RecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender)
{
  NS_LOG_FUNCTION (this << " src " << sender);
  RRHeader rrepHeader;
  p->RemoveHeader (rrepHeader);
  Ipv4Address dst = rrepHeader.GetDst ();
  NS_LOG_LOGIC ("RREP destination " << dst << " RREP origin " << rrepHeader.GetOrigin ());


  // If RREP is Hello message
//  if (dst == rrepHeader.GetOrigin ())
//    {
//      ProcessHello (rrepHeader, receiver);
//      return;
//    }

  /*
   * If the route table entry to the destination is created or updated, then the following actions occur:
   * -  the route is marked as active,
   * -  the destination sequence number is marked as valid,
   * -  the next hop in the route entry is assigned to be the node from which the RREP is received,
   *    which is indicated by the source IP address field in the IP header,
   * -  the hop count is set to the value of the hop count from RREP message + 1
   * -  the expiry time is set to the current time plus the value of the Lifetime in the RREP message,
   * -  and the destination sequence number is the Destination Sequence Number in the RREP message.
   */

      // The forward route for this destination is created if it does not already exist.
      NS_LOG_LOGIC ("add new route");

      //m_routingtable.AddRoute (newEntry);

  if (IsMyOwnAddress (rrepHeader.GetOrigin ()))
    {
      Ipv4Route toDst;

      m_routingtable.LookupRoute (dst, toDst);
      SendPacketFromQueue (dst, &toDst);
      return;
    }
}


void
RoutingProtocol::RecvAodv (Ptr<Socket> socket)
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

  TypeHeader tHeader (SDSNTYPE_RREQ);
  packet->RemoveHeader (tHeader);
  if (!tHeader.IsValid ())
    {
      NS_LOG_DEBUG ("SDSN message " << packet->GetUid () << " with unknown type received: " << tHeader.Get () << ". Drop");
      return; // drop
    }
  switch (tHeader.Get ())
    {
    case SDSNTYPE_RREQ:
      {
        RecvRequest (packet, receiver, sender);
        break;
      }
    case SDSNTYPE_RREP:
      {
        RecvReply (packet, receiver, sender);
        break;
      }
    case SDSNTYPE_RERR:
      {
        //RecvError (packet, sender);
        break;
      }
    case SDSNTYPE_RREP_ACK:
      {
        //RecvReplyAck (sender);
        break;
      }
    }
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
  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvAodv, this));
  socket->BindToNetDevice (l3->GetNetDevice (i));
  socket->Bind (InetSocketAddress (iface.GetLocal (), SDSN_PORT));
  socket->SetAllowBroadcast (false);
  socket->SetIpRecvTtl (true);
  m_socketAddresses.insert (std::make_pair (socket, iface));

  // create also a subnet broadcast socket
//  socket = Socket::CreateSocket (GetObject<Node> (),
//                                 UdpSocketFactory::GetTypeId ());
//  NS_ASSERT (socket != 0);
//  socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvAodv, this));
//  socket->BindToNetDevice (l3->GetNetDevice (i));
//  socket->Bind (InetSocketAddress (iface.GetBroadcast (), SDSN_PORT));
//  socket->SetAllowBroadcast (true);
//  socket->SetIpRecvTtl (true);
//  m_socketSubnetBroadcastAddresses.insert (std::make_pair (socket, iface));

  // Add local broadcast record to the routing table
//  Ptr<NetDevice> dev = m_ipv4->GetNetDevice (m_ipv4->GetInterfaceForAddress (iface.GetLocal ()));
//  RoutingTableEntry rt (/*device=*/ dev, /*dst=*/ iface.GetBroadcast (), /*know seqno=*/ true, /*seqno=*/ 0, /*iface=*/ iface,
//                                    /*hops=*/ 1, /*next hop=*/ iface.GetBroadcast (), /*lifetime=*/ Simulator::GetMaximumSimulationTime ());
//  m_routingTable.AddRoute (rt);

//  if (l3->GetInterface (i)->GetArpCache ())
//    {
//      m_nb.AddArpCache (l3->GetInterface (i)->GetArpCache ());
//    }
//
//  // Allow neighbor manager use this interface for layer 2 feedback if possible
//  Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
//  if (wifi == 0)
//    {
//      return;
//    }
//  Ptr<WifiMac> mac = wifi->GetMac ();
//  if (mac == 0)
//    {
//      return;
//    }
//
//  mac->TraceConnectWithoutContext ("DroppedMpdu", MakeCallback (&RoutingProtocol::NotifyTxError, this));
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
      NS_LOG_LOGIC ("No sdsn interfaces");

      return;
    }
  //m_routingtable.DeleteAllRoutesFromInterface (m_ipv4->GetAddress (i, 0));

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
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvAodv,this));
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetLocal (), SDSN_PORT));
          socket->SetAllowBroadcast (true);
          m_socketAddresses.insert (std::make_pair (socket, iface));
        }
    }
  else
    {
      NS_LOG_LOGIC ("SDSN does not work with more then one address per each interface. Ignore added address");
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
          socket->SetRecvCallback (MakeCallback (&RoutingProtocol::RecvAodv, this));
          // Bind to any IP address so that broadcasts can be received
          socket->BindToNetDevice (l3->GetNetDevice (i));
          socket->Bind (InetSocketAddress (iface.GetLocal (), SDSN_PORT));
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
RoutingProtocol::Start ()
{
  NS_LOG_FUNCTION (this);
//  if (m_enableHello)
//    {
//      m_nb.ScheduleTimer ();
//    }
//  m_rreqRateLimitTimer.SetFunction (&RoutingProtocol::RreqRateLimitTimerExpire,
//                                    this);
//  m_rreqRateLimitTimer.Schedule (Seconds (1));
//
//  m_rerrRateLimitTimer.SetFunction (&RoutingProtocol::RerrRateLimitTimerExpire,
//                                    this);
//  m_rerrRateLimitTimer.Schedule (Seconds (1));

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
  // Remember lo route
//  RoutingTableEntry rt (/*device=*/ m_lo, /*dst=*/ Ipv4Address::GetLoopback (), /*know seqno=*/ true, /*seqno=*/ 0,
//                                    /*iface=*/ Ipv4InterfaceAddress (Ipv4Address::GetLoopback (), Ipv4Mask ("255.0.0.0")),
//                                    /*hops=*/ 1, /*next hop=*/ Ipv4Address::GetLoopback (),
//                                    /*lifetime=*/ Simulator::GetMaximumSimulationTime ());
  Ipv4Route rt;
  rt.SetOutputDevice(m_lo);
  rt.SetDestination(Ipv4Address::GetLoopback());
  rt.SetGateway(Ipv4Address::GetLoopback());

  m_routingtable.Add (rt);

  Simulator::ScheduleNow (&RoutingProtocol::Start, this);
}

void
RoutingProtocol::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{

}

void
NetView::SetControlDomain(Ptr<Node> con, NodeContainer swc)
{
  for(uint32_t i = 0; i < swc.GetN(); i++)
    {
      SetPair(con, swc.Get(i));
    }
}

void
NetView::ControlPathRouting(Ptr<Node> swc, std::vector<std::pair<Ptr<Node>,NetViewEdge>> control_path)
{
  //m_ConIp

  Ptr<NetDevice> con_dev =  control_path.back().second.GetDevice(1);

  Ptr<Ipv4> con_ipv4 = con_dev->GetNode()->GetObject<Ipv4>();
  int32_t con_interface = con_ipv4->GetInterfaceForDevice(con_dev);
  Ipv4Address con_ip = con_ipv4->GetAddress(con_interface, 0).GetLocal();

  Ptr<RoutingProtocol> rp = swc->GetObject<RoutingProtocol>();
  rp->SetConIp(con_ip);

  //Route To Con

  Ipv4Route ro_to_con;

  ro_to_con.SetDestination(con_ip);


  Ptr<NetDevice> src_dev = control_path.front().second.GetDevice(2);
  Ptr<Ipv4> src_ipv4 = control_path.front().first->GetObject<Ipv4>();
  int32_t src_interface = src_ipv4->GetInterfaceForDevice(src_dev);
  Ipv4Address src_ip = src_ipv4->GetAddress(src_interface, 0).GetLocal();
  ro_to_con.SetSource(src_ip);

  Ipv4Route ro_to_swc;
  ro_to_swc.SetDestination(src_ip);
  ro_to_swc.SetSource(con_ip);


  Ptr<Ipv4> uphop_ipv4;
  Ptr<RoutingProtocol> path_rp;
  int32_t uphop_interface;
  Ipv4Address gate_way;
  Ptr<Ipv4> cur_ipv4;
  int32_t cur_interface;


  for(auto it = control_path.begin(); it != control_path.end(); it++ )
    {
      //route to con;
      path_rp = it->first->GetObject<RoutingProtocol>();
      if(it->first == control_path.back().first)
        {
          uphop_ipv4 = it->second.GetDevice(1)->GetNode()->GetObject<Ipv4>();
        }
      else
        {
          uphop_ipv4 = (it+1)->first->GetObject<Ipv4>();
        }
      uphop_interface = uphop_ipv4->GetInterfaceForDevice(it->second.GetDevice(1));
      gate_way = uphop_ipv4->GetAddress(uphop_interface, 0).GetLocal();

      ro_to_con.SetGateway(gate_way);
      ro_to_con.SetOutputDevice(it->second.GetDevice(2));
      path_rp->AddRoute(ro_to_con);

      //route to swc;
      cur_ipv4 = it->first->GetObject<Ipv4>();
      cur_interface =  cur_ipv4->GetInterfaceForDevice(it->second.GetDevice(2));
      ro_to_swc.SetGateway(cur_ipv4->GetAddress(cur_interface, 0).GetLocal());
      ro_to_swc.SetOutputDevice(it->second.GetDevice(1));
      uphop_ipv4->GetObject<RoutingProtocol>()->AddRoute(ro_to_swc);
    }
}

void
NetView::SetPair(Ptr<Node> con, Ptr<Node> swc)
{
  Ptr<RoutingProtocol> crp = con->GetObject<RoutingProtocol>();
  Ptr<RoutingProtocol> srp = swc->GetObject<RoutingProtocol>();


  std::queue<std::pair<Ptr<Node>,NetViewEdge>> bfs;
  std::map<Ptr<Node>,std::pair<Ptr<Node>,NetViewEdge>> Traversed; //Noted Node , Imported by

  bfs.push(*(m_view.find(con)->second.begin()));
  Traversed.insert(std::make_pair(con,*(m_view.find(con)->second.begin())));


  while (bfs.front().first != swc && !bfs.empty())
    {
      for(auto it = ++m_view.find(bfs.front().first)->second.begin();it != m_view.find(bfs.front().first)->second.end(); it++)
        {
          if(Traversed.count(it->first)) continue;
          bfs.push(*it);
          Traversed.insert(std::make_pair(it->first, *(m_view.find(bfs.front().first)->second.begin())));
        }
      bfs.pop();
    }

  std::vector<std::pair<Ptr<Node>,NetViewEdge>> control_path;

  Ptr<Node> temp = swc;
  while(temp != con)
    {
      Ptr<Node> up_hop = Traversed.find(temp)->second.first;


      for(auto it = m_view.find(up_hop)->second.begin();it != m_view.find(up_hop)->second.end(); it++)
        {
          if(it->first == temp)
            {
              control_path.push_back(*it);
              temp = up_hop;
              break;
            }
        }

//      control_path.push_back(Traversed.find(temp)->second);
//      temp = Traversed.find(temp)->second.first;
    }

  ControlPathRouting(swc,control_path);

  crp->SetNodeType(CONTROLLER);
  crp->SetNodeState(CONNECTED);
  srp->SetNodeType(SWITCH);
  srp->SetNodeState(CONNECTED);

}



}




}

/* ... */


