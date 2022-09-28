#include "sdn-packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SDNPACKET");

namespace sdn {

TypeHeader::TypeHeader (MessageType t = SDNTYPE_RREQ)
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
}


}
