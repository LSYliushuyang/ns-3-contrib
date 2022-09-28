#ifndef SDNPACKET_H
#define SDNPACKET_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/enum.h"
#include "ns3/nstime.h"
#include <iostream>


namespace ns3 {

namespace sdn {

enum MessageType
{
  SDNTYPE_RREQ  = 1,
  SDNTYPE_RREP  = 2,
  SDNTYPE_HELLO = 3,
  SDNTYPE_CONFIG = 4
};

class TypeHeader : public Header
{
public:
  /**
   * constructor
   * \param t the AODV RREQ type
   */
  TypeHeader (MessageType t = SDNTYPE_RREQ);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;

  uint32_t GetSerializedSize () const;

  void Serialize (Buffer::Iterator start) const;

  uint32_t Deserialize (Buffer::Iterator start);

  void Print (std::ostream &os) const;
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
* \ingroup aodv
* \brief   Route Request (RREQ) Message Format
  \verbatim
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |     Type      |J|R|G|D|U|   Reserved          |  Time         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                            RREQ ID                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Destination IP Address                     |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  Destination Sequence Number                  |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                    Originator IP Address                      |   //Switch's IP
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  Originator Sequence Number                   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  \endverbatim
*/



class ControlPacketHeader : public Header
{
public:
  /**
   * constructor
   *
   * \param flags the message flags (0)
   * \param reserved the reserved bits (0)
   * \param hopCount the hop count
   * \param requestID the request ID
   * \param dst the destination IP address
   * \param dstSeqNo the destination sequence number
   * \param origin the origin IP address
   * \param originSeqNo the origin sequence number
   */
   ControlPacketHeader (uint8_t flags = 0, uint8_t reserved = 0, Time time = Seconds(0),
              uint32_t requestID = 0, Ipv4Address dst = Ipv4Address (),
              uint32_t dstSeqNo = 0, Ipv4Address origin = Ipv4Address (),
              uint32_t originSeqNo = 0);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  /**
   * \brief Set the hop count
   * \param count the hop count
   */
  void SetTime (Time time)
  {
    m_time = time;
  }
  /**
   * \brief Get the hop count
   * \return the hop count
   */
  uint8_t GetTime () const
  {
    return m_time;
  }
  /**
   * \brief Set the request ID
   * \param id the request ID
   */
  void SetId (uint32_t id)
  {
    m_requestID = id;
  }
  /**
   * \brief Get the request ID
   * \return the request ID
   */
  uint32_t GetId () const
  {
    return m_requestID;
  }
  /**
   * \brief Set the destination address
   * \param a the destination address
   */
  void SetDst (Ipv4Address a)
  {
    m_dst = a;
  }
  /**
   * \brief Get the destination address
   * \return the destination address
   */
  Ipv4Address GetDst () const
  {
    return m_dst;
  }
  /**
   * \brief Set the destination sequence number
   * \param s the destination sequence number
   */
  void SetDstSeqno (uint32_t s)
  {
    m_dstSeqNo = s;
  }
  /**
   * \brief Get the destination sequence number
   * \return the destination sequence number
   */
  uint32_t GetDstSeqno () const
  {
    return m_dstSeqNo;
  }
  /**
   * \brief Set the origin address
   * \param a the origin address
   */
  void SetOrigin (Ipv4Address a)
  {
    m_origin = a;
  }
  /**
   * \brief Get the origin address
   * \return the origin address
   */
  Ipv4Address GetOrigin () const
  {
    return m_origin;
  }
  /**
   * \brief Set the origin sequence number
   * \param s the origin sequence number
   */
  void SetOriginSeqno (uint32_t s)
  {
    m_originSeqNo = s;
  }
  /**
   * \brief Get the origin sequence number
   * \return the origin sequence number
   */
  uint32_t GetOriginSeqno () const
  {
    return m_originSeqNo;
  }

  // Flags
  /**
   * \brief Set the gratuitous RREP flag
   * \param f the gratuitous RREP flag
   */
  void SetGratuitousRrep (bool f);
  /**
   * \brief Get the gratuitous RREP flag
   * \return the gratuitous RREP flag
   */
  bool GetGratuitousRrep () const;
  /**
   * \brief Set the Destination only flag
   * \param f the Destination only flag
   */
  void SetDestinationOnly (bool f);
  /**
   * \brief Get the Destination only flag
   * \return the Destination only flag
   */
  bool GetDestinationOnly () const;
  /**
   * \brief Set the unknown sequence number flag
   * \param f the unknown sequence number flag
   */
  void SetUnknownSeqno (bool f);
  /**
   * \brief Get the unknown sequence number flag
   * \return the unknown sequence number flag
   */
  bool GetUnknownSeqno () const;

  /**
   * \brief Comparison operator
   * \param o RREQ header to compare
   * \return true if the RREQ headers are equal
   */
  bool operator== (ControlPacketHeader const & o) const;
private:
  uint8_t        m_flags;          ///< |J|R|G|D|U| bit flags, see RFC
  uint8_t        m_reserved;       ///< Not used (must be 0)
  Time           m_time;
  uint32_t       m_requestID;      ///< RREQ ID
  Ipv4Address    m_dst;            ///< Destination IP Address
  uint32_t       m_dstSeqNo;       ///< Destination Sequence Number
  Ipv4Address    m_origin;         ///< Originator IP Address
  uint32_t       m_originSeqNo;    ///< Source Sequence Number
};

/**
  * \brief Stream output operator
  * \param os output stream
  * \return updated stream
  */
std::ostream & operator<< (std::ostream & os, ControlPacketHeader const &);

}



}


#endif
