/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef SDSN_H
#define SDSN_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include <list>
#include <vector>
#include <queue>

namespace ns3 {

namespace sdsn
{

enum NodeType {SWITCH, CONTROLLER};

enum NodeState {CONNECTED, FREE};

class QueueEntry
{
public:
  /// IPv4 routing unicast forward callback typedef
  typedef Ipv4RoutingProtocol::UnicastForwardCallback UnicastForwardCallback;
  /// IPv4 routing error callback typedef
  typedef Ipv4RoutingProtocol::ErrorCallback ErrorCallback;
  /**
   * constructor
   *
   * \param pa the packet to add to the queue
   * \param h the Ipv4Header
   * \param ucb the UnicastForwardCallback function
   * \param ecb the ErrorCallback function
   * \param exp the expiration time
   */
  QueueEntry (Ptr<const Packet> pa = 0, Ipv4Header const & h = Ipv4Header (),
              UnicastForwardCallback ucb = UnicastForwardCallback (),
              ErrorCallback ecb = ErrorCallback (), Time exp = Simulator::Now ())
    : m_packet (pa),
      m_header (h),
      m_ucb (ucb),
      m_ecb (ecb),
      m_expire (exp + Simulator::Now ())
  {
  }

  /**
   * \brief Compare queue entries
   * \param o QueueEntry to compare
   * \return true if equal
   */
  bool operator== (QueueEntry const & o) const
  {
    return ((m_packet == o.m_packet) && (m_header.GetDestination () == o.m_header.GetDestination ()) && (m_expire == o.m_expire));
  }

  // Fields
  /**
   * Get unicast forward callback
   * \returns unicast callback
   */
  UnicastForwardCallback GetUnicastForwardCallback () const
  {
    return m_ucb;
  }
  /**
   * Set unicast forward callback
   * \param ucb The unicast callback
   */
  void SetUnicastForwardCallback (UnicastForwardCallback ucb)
  {
    m_ucb = ucb;
  }
  /**
   * Get error callback
   * \returns the error callback
   */
  ErrorCallback GetErrorCallback () const
  {
    return m_ecb;
  }
  /**
   * Set error callback
   * \param ecb The error callback
   */
  void SetErrorCallback (ErrorCallback ecb)
  {
    m_ecb = ecb;
  }
  /**
   * Get packet from entry
   * \returns the packet
   */
  Ptr<const Packet> GetPacket () const
  {
    return m_packet;
  }
  /**
   * Set packet in entry
   * \param p The packet
   */
  void SetPacket (Ptr<const Packet> p)
  {
    m_packet = p;
  }
  /**
   * Get IPv4 header
   * \returns the IPv4 header
   */
  Ipv4Header GetIpv4Header () const
  {
    return m_header;
  }
  /**
   * Set IPv4 header
   * \param h the IPv4 header
   */
  void SetIpv4Header (Ipv4Header h)
  {
    m_header = h;
  }
  /**
   * Set expire time
   * \param exp The expiration time
   */
  void SetExpireTime (Time exp)
  {
    m_expire = exp + Simulator::Now ();
  }
  /**
   * Get expire time
   * \returns the expiration time
   */
  Time GetExpireTime () const
  {
    return m_expire - Simulator::Now ();
  }

private:
  /// Data packet
  Ptr<const Packet> m_packet;
  /// IP header
  Ipv4Header m_header;
  /// Unicast forward callback
  UnicastForwardCallback m_ucb;
  /// Error callback
  ErrorCallback m_ecb;
  /// Expire time for queue entry
  Time m_expire;
};

class RequestQueue
{
public:
  /**
   * constructor
   *
   * \param maxLen the maximum length
   * \param routeToQueueTimeout the route to queue timeout
   */
  RequestQueue (uint32_t maxLen, Time routeToQueueTimeout)
    : m_maxLen (maxLen),
      m_queueTimeout (routeToQueueTimeout)
  {
  }
  /**
   * Push entry in queue, if there is no entry with the same packet and destination address in queue.
   * \param entry the queue entry
   * \returns true if the entry is queued
   */
  bool Enqueue (QueueEntry & entry)
  {
    for (std::vector<QueueEntry>::const_iterator i = m_queue.begin (); i
         != m_queue.end (); ++i)
      {
        if ((i->GetPacket ()->GetUid () == entry.GetPacket ()->GetUid ())
            && (i->GetIpv4Header ().GetDestination ()
                == entry.GetIpv4Header ().GetDestination ()))
          {
            return false;
          }
      }
    entry.SetExpireTime (m_queueTimeout);
    if (m_queue.size () == m_maxLen)
      {
        Drop (m_queue.front (), "Drop the most aged packet"); // Drop the most aged packet
        m_queue.erase (m_queue.begin ());
      }
    m_queue.push_back (entry);
    return true;
  }
  /**
   * Return first found (the earliest) entry for given destination
   *
   * \param dst the destination IP address
   * \param entry the queue entry
   * \returns true if the entry is dequeued
   */
  bool Dequeue (Ipv4Address dst, QueueEntry & entry)
  {
    for (std::vector<QueueEntry>::iterator i = m_queue.begin (); i != m_queue.end (); ++i)
      {
        if (i->GetIpv4Header ().GetDestination () == dst)
          {
            entry = *i;
            m_queue.erase (i);
            return true;
          }
      }
    return false;
  }
  /**
   * Remove all packets with destination IP address dst
   * \param dst the destination IP address
   */
  void DropPacketWithDst (Ipv4Address dst);
  /**
   * Finds whether a packet with destination dst exists in the queue
   *
   * \param dst the destination IP address
   * \returns true if an entry with the IP address is found
   */
  bool Find (Ipv4Address dst);
  /**
   * \returns the number of entries
   */
  uint32_t GetSize ();

  // Fields
  /**
   * Get maximum queue length
   * \returns the maximum queue length
   */
  uint32_t GetMaxQueueLen () const
  {
    return m_maxLen;
  }
  /**
   * Set maximum queue length
   * \param len The maximum queue length
   */
  void SetMaxQueueLen (uint32_t len)
  {
    m_maxLen = len;
  }
  /**
   * Get queue timeout
   * \returns the queue timeout
   */
  Time GetQueueTimeout () const
  {
    return m_queueTimeout;
  }
  /**
   * Set queue timeout
   * \param t The queue timeout
   */
  void SetQueueTimeout (Time t)
  {
    m_queueTimeout = t;
  }

private:
  /// The queue
  std::vector<QueueEntry> m_queue;
  /// Remove all expired entries
  void Purge ();
  /**
   * Notify that packet is dropped from queue by timeout
   * \param en the queue entry to drop
   * \param reason the reason to drop the entry
   */
  void Drop (QueueEntry en, std::string reason)
  {
    //NS_LOG_LOGIC (reason << en.GetPacket ()->GetUid () << " " << en.GetIpv4Header ().GetDestination ());
    en.GetErrorCallback () (en.GetPacket (), en.GetIpv4Header (),
                            Socket::ERROR_NOROUTETOHOST);
    return;
  }
  /// The maximum number of packets that we allow a routing protocol to buffer.
  uint32_t m_maxLen;
  /// The maximum period of time that a routing protocol is allowed to buffer a packet for, seconds.
  Time m_queueTimeout;
};

class RRHeader : public Header
{
public:
  RRHeader(){};
  static TypeId GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::sdsn::RRHeader")
        .SetParent<Header>()
        .SetGroupName("sdsn")
        .AddConstructor<RRHeader>()
        ;
    return tid;
  }

  TypeId
  GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  uint32_t GetSerializedSize () const
  {
    return 8;
  }
  void Serialize (Buffer::Iterator start) const
  {
    WriteTo(start,m_destination);
    WriteTo(start,m_origin);
  }
  uint32_t Deserialize (Buffer::Iterator start)
  {
    Buffer::Iterator i = start;
    ReadFrom(i,m_destination);
    ReadFrom(i,m_origin);
    uint32_t dist = i.GetDistanceFrom (start);
    NS_ASSERT (dist == GetSerializedSize ());
    return dist;
  }
  void Print (std::ostream &os) const
  {
    os << " destination: ipv4 " << m_destination
       << " source: ipv4 "  << m_origin ;
  }

public:
  void SetDst(Ipv4Address add)
  {
    m_destination = add;
  }
  void SetOriginAdd(Ipv4Address add)
  {
    m_origin = add;
  }

  Ipv4Address GetDst() const
  {
    return m_destination;
  }

  Ipv4Address GetOrigin()
  {
    return m_origin;
  }

private:
  Ipv4Address m_destination;
  Ipv4Address m_origin;
};


enum MessageType
{
  SDSNTYPE_RREQ  = 1,   //!< AODVTYPE_RREQ
  SDSNTYPE_RREP  = 2,   //!< AODVTYPE_RREP
  SDSNTYPE_RERR  = 3,   //!< AODVTYPE_RERR
  SDSNTYPE_RREP_ACK = 4 //!< AODVTYPE_RREP_ACK
};

class TypeHeader : public Header
{
public:
  /**
   * constructor
   * \param t the AODV RREQ type
   */
  TypeHeader (MessageType t = SDSNTYPE_RREQ)
  : m_type (t),
  m_valid (true)
{
}

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ()
  {
    {
      static TypeId tid = TypeId ("ns3::sdsn::TypeHeader")
        .SetParent<Header> ()
        .SetGroupName ("sdsn")
        .AddConstructor<TypeHeader> ()
      ;
      return tid;
    }
  }

  TypeId GetInstanceTypeId () const
  {
    return GetTypeId ();
  }

  uint32_t GetSerializedSize () const
  {
    return 1;
  }

  void Serialize (Buffer::Iterator start) const
  {
    start.WriteU8 ((uint8_t) m_type);
  }

  uint32_t Deserialize (Buffer::Iterator start)
  {
    Buffer::Iterator i = start;
    uint8_t type = i.ReadU8 ();
    m_valid = true;
    switch (type)
      {
      case SDSNTYPE_RREQ:
      case SDSNTYPE_RREP:
      case SDSNTYPE_RERR:
      case SDSNTYPE_RREP_ACK:
        {
          m_type = (MessageType) type;
          break;
        }
      default:
        m_valid = false;
      }
    uint32_t dist = i.GetDistanceFrom (start);
    NS_ASSERT (dist == GetSerializedSize ());
    return dist;
  }

  void Print (std::ostream &os) const
  {
    switch (m_type)
      {
      case SDSNTYPE_RREQ:
        {
          os << "RREQ";
          break;
        }
      case SDSNTYPE_RREP:
        {
          os << "RREP";
          break;
        }
      case SDSNTYPE_RERR:
        {
          os << "RERR";
          break;
        }
      case SDSNTYPE_RREP_ACK:
        {
          os << "RREP_ACK";
          break;
        }
      default:
        os << "UNKNOWN_TYPE";
      }
  }

  /**
   * \returns the type
   */
  MessageType Get () const
  {
    return m_type;
  }
  /**
   * Check that type if valid
   * \returns true if the type is valid
   */
  bool IsValid () const
  {
    return m_valid;
  }
  /**
   * \brief Comparison operator
   * \param o header to compare
   * \return true if the headers are equal
   */
  bool operator== (TypeHeader const & o) const
    {
      return (m_type == o.m_type && m_valid == o.m_valid);
    }

private:
  MessageType m_type; ///< type of the message
  bool m_valid; ///< Indicates if the message is valid
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */



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
    static TypeId tid = TypeId ("ns3::sdsn::DeferredRouteOutputTag")
      .SetParent<Tag> ()
      .SetGroupName ("sdsn")
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

class NetViewEdge
{
public:
  NetViewEdge(Ptr<NetDevice> dev1,Ptr<NetDevice> dev2,Ptr<Channel> cha)
  {
    m_dev1=dev1;
    m_dev2=dev2;
    m_cha=cha;
  }

  NetViewEdge()
  : m_dev1(0),m_cha(0),m_dev2(0)
  {
  }

  Time GetDelay()
  {
    return m_cha->GetObject<PointToPointWirelessChannel>()->GetDelay();
  }

  double GetLoad()
  {
    Ptr<Queue<Packet>> que = m_dev1->GetObject<PointToPointWirelessNetDevice>()->GetQueue();
    return (que->GetCurrentSize().GetValue())/(que->GetMaxSize().GetValue());
  }

  Ptr<NetDevice> GetDevice(int dev){return dev == 1 ? m_dev1 : m_dev2;}

private:
  Ptr<NetDevice> m_dev1;
  Ptr<Channel> m_cha;
  Ptr<NetDevice> m_dev2;
};

class NodeInfo
{
public:
  NodeInfo():m_delay(Seconds(9999)),m_load(1){}
  void SetDelay(Time time){m_delay = time;}
  void SetLoad(uint32_t load){m_load = load;}

  Time GetDelay(){return m_delay;}
  uint32_t GetLoad(){return m_load;}

private:
  Time m_delay;
  uint32_t m_load;
};

class NetView
{
public:
  NetView();
  void Install(NodeContainer c);
  std::pair<Time,Time> GetDelay(Ptr<Node> node1, Ptr<Node> node2);
  std::pair<Time,double> GetLoad(Ptr<Node> node1, Ptr<Node> node2);

  void SetControlDomain(Ptr<Node> con, NodeContainer swc);

  //void SetNodeType(Ptr<Node> node, NodeType type);

  NetView& operator=(const NetView& tmp)
  {
    if(this == &tmp)
      {
        return *this;
      }
    m_view = tmp.m_view;
    return *this;
  }

  void InitSnap();
  void UpdateSnap(Ptr<Node> master, Ptr<Node> slave, Time time ,Time delay,uint32_t load);

//  void SetUpdate(Callback<void,Time> func) {m_uddelay_func = func;}
//  void SetUpdate(Callback<void,uint32_t> func) {m_udload_func = func;}

private:

  typedef std::map<Ptr<Node>,std::list<std::pair<Ptr<Node>,NetViewEdge>>> Adjlist;
  typedef std::list<std::pair<Ptr<Node>,NetViewEdge>> LList;
  Adjlist m_view;


//  std::map<Ptr<Node>,std::map<Ptr<Node>,std::pair<Time,NodeInfo>>> m_snap;
  std::map<std::pair<Ptr<Node>,Ptr<Node>>,std::pair<Time,NodeInfo>> m_snap;

//
//  Callback<void,Time> m_uddelay_func;
//  Callback<void,uint32_t> m_udload_func;

private:

  void SetPair(Ptr<Node> con, Ptr<Node> swc);
  void ControlPathRouting(Ptr<Node> swc, std::vector<std::pair<Ptr<Node>,NetViewEdge>> control_path);
};

class RoutingTable
{
public:
  RoutingTable(){}
  void Add(Ipv4Route rte)
  {
    if(!m_table.count(rte.GetDestination()))
      {
        m_table.insert(std::make_pair(rte.GetDestination(),rte));
      }
  }
  void Delete(Ipv4Address add)
  {
    if(m_table.count(add))
      {
        m_table.erase(add);
      }
  }
  bool LookupRoute (Ipv4Address id, Ipv4Route & rt);
private:
  std::map<Ipv4Address,Ipv4Route> m_table;
};

class RoutingProtocol : public Ipv4RoutingProtocol
{
public:
  static const uint32_t SDSN_PORT;

  static NetView LogCen;

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
  void AddRoute(Ipv4Route rte)
  {
    m_routingtable.Add(rte);
  }

  void DeleteRoute(Ipv4Address add)
  {
    m_routingtable.Delete(add);
  }

  NodeType GetNodeType(){return m_type;}
  NodeState GetNodeState(){return m_state;}

  void SetNodeType(NodeType nt){m_type = nt;}
  void SetNodeState(NodeState ns){m_state = ns;}

  RoutingProtocol()
  :m_queue(100,Seconds(30)),m_type(SWITCH),m_state(FREE),m_interval(Seconds(1))
  {
    m_htimer.SetFunction (&RoutingProtocol::HelloTimerExpire, this);
    m_htimer.Schedule (Seconds(0));
  };

  void RecvAodv (Ptr<Socket> socket);

  void SetConIp(Ipv4Address add){m_conIP = add;}
  Ipv4Address GetConIP(){return m_conIP;}

  void SetOutSocket(Ipv4InterfaceAddress add){m_outSocket = std::make_pair(FindSocketWithInterfaceAddress(add),add);}

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

  void SendReply (RRHeader const & rreqHeader, Ipv4Route const & toOrigin);

  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress addr ) const;

  void RecvReply (Ptr<Packet> p, Ipv4Address receiver, Ipv4Address sender);

  void SendPacketFromQueue (Ipv4Address dst, Ptr<Ipv4Route> route);

  void Start ();

  void SendHello ();


private:
  RoutingTable m_routingtable;
  Ptr<NetDevice> m_lo;
  std::map< Ptr<Socket>, Ipv4InterfaceAddress > m_socketAddresses;
  Ptr<Ipv4> m_ipv4;
  RequestQueue m_queue;

  NodeType m_type;
  NodeState m_state;

  Ipv4Address m_conIP;
  std::pair<Ptr<Socket>,Ipv4InterfaceAddress> m_outSocket;

  Timer m_htimer;
  Time m_interval;




};




}

/* ... */

}

#endif /* SDSN_H */

