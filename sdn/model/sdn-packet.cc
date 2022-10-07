#include "sdn-packet.h"
#include "ns3/address-utils.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SDNPACKET");

namespace sdn {
NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t)
: m_type (t),
m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  {
    static TypeId tid = TypeId ("ns3::ssn::TypeHeader")
      .SetParent<Header> ()
      .SetGroupName ("SDN")
      .AddConstructor<TypeHeader> ()
    ;
    return tid;
  }
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
TypeHeader::GetSerializedSize () const
{
  return 1;
}

void
TypeHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  m_valid = true;
  switch (type)
    {
    case SDNTYPE_RREQ:
    case SDNTYPE_RREP:
    case SDNTYPE_HELLO:
    case SDNTYPE_CONFIG:
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

void
TypeHeader::Print (std::ostream &os) const
{
  switch (m_type)
    {
    case SDNTYPE_RREQ:
      {
        os << "RREQ";
        break;
      }
    case SDNTYPE_RREP:
      {
        os << "RREP";
        break;
      }
    case SDNTYPE_HELLO:
      {
        os << "HELLO";
        break;
      }
    case SDNTYPE_CONFIG:
      {
        os << "CONFIG";
        break;
      }
    default:
      os << "UNKNOWN_TYPE";
    }
}

MessageType
TypeHeader::Get () const
{
  return m_type;
}
/**
 * Check that type if valid
 * \returns true if the type is valid
 */
bool
TypeHeader::IsValid () const
{
  return m_valid;
}
/**
 * \brief Comparison operator
 * \param o header to compare
 * \return true if the headers are equal
 */
bool
TypeHeader::operator== (TypeHeader const & o) const
  {
    return (m_type == o.m_type && m_valid == o.m_valid);
  }





NS_OBJECT_ENSURE_REGISTERED (ControlPacketHeader);

ControlPacketHeader::ControlPacketHeader (uint8_t flags, uint8_t reserved, uint32_t requestID, Ipv4Address dst,
                                          uint32_t dstSeqNo, Ipv4Address origin, uint32_t originSeqNo)
                    : m_flags (flags),
                      m_reserved (reserved),
                      m_requestID (requestID),
                      m_dst (dst),
                      m_dstSeqNo (dstSeqNo),
                      m_origin (origin),
                      m_originSeqNo (originSeqNo)
                  {
                  }

TypeId
ControlPacketHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::sdn::RreqHeader")
    .SetParent<Header> ()
    .SetGroupName ("SDN")
    .AddConstructor<ControlPacketHeader> ()
  ;
  return tid;
}

TypeId
ControlPacketHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
ControlPacketHeader::GetSerializedSize () const
{
  return 23;
}

void
ControlPacketHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_flags);
  i.WriteU8 (m_reserved);
  i.WriteU8 (m_reserved);
  i.WriteHtonU32 (m_requestID);
  WriteTo (i, m_dst);
  i.WriteHtonU32 (m_dstSeqNo);
  WriteTo (i, m_origin);
  i.WriteHtonU32 (m_originSeqNo);
}

uint32_t
ControlPacketHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_flags = i.ReadU8 ();
  m_reserved = i.ReadU8 ();
  i.ReadU8 ();
  m_requestID = i.ReadNtohU32 ();
  ReadFrom (i, m_dst);
  m_dstSeqNo = i.ReadNtohU32 ();
  ReadFrom (i, m_origin);
  m_originSeqNo = i.ReadNtohU32 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
ControlPacketHeader::Print (std::ostream &os) const
{
  os << "RREQ ID " << m_requestID << " destination: ipv4 " << m_dst
     << " sequence number " << m_dstSeqNo << " source: ipv4 "
     << m_origin << " sequence number " << m_originSeqNo
     << " flags:" << " Gratuitous RREP " << (*this).GetGratuitousRrep ()
     << " Destination only " << (*this).GetDestinationOnly ()
     << " Unknown sequence number " << (*this).GetUnknownSeqno ();
}

void
ControlPacketHeader::SetGratuitousRrep (bool f)
{
  if (f)
    {
      m_flags |= (1 << 5);
    }
  else
    {
      m_flags &= ~(1 << 5);
    }
}

bool
ControlPacketHeader::GetGratuitousRrep () const
{
  return (m_flags & (1 << 5));
}

void
ControlPacketHeader::SetDestinationOnly (bool f)
{
  if (f)
    {
      m_flags |= (1 << 4);
    }
  else
    {
      m_flags &= ~(1 << 4);
    }
}

bool
ControlPacketHeader::GetDestinationOnly () const
{
  return (m_flags & (1 << 4));
}

void
ControlPacketHeader::SetUnknownSeqno (bool f)
{
  if (f)
    {
      m_flags |= (1 << 3);
    }
  else
    {
      m_flags &= ~(1 << 3);
    }
}

bool
ControlPacketHeader::GetUnknownSeqno () const
{
  return (m_flags & (1 << 3));
}

bool
ControlPacketHeader::operator== (ControlPacketHeader const & o) const
{
  return (m_flags == o.m_flags && m_reserved == o.m_reserved
          && m_requestID == o.m_requestID
          && m_dst == o.m_dst && m_dstSeqNo == o.m_dstSeqNo
          && m_origin == o.m_origin && m_originSeqNo == o.m_originSeqNo);
}



}


}
