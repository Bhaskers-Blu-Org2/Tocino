/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 1997, 1998 Carnegie Mellon University.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: The AODV code developed by the CMU/MONARCH group was optimized and
 * tuned by Samir Das and Mahesh Marina, University of Cincinnati.
 * The work was partially done in Sun Microsystems.
 *
 * Ported to ns-3 by Elena Borovkova <borovkovaes@iitp.ru>
 */
#include "aodv-packet.h"
#include "ns3/address-utils.h"

namespace ns3 {
namespace aodv {

//-----------------------------------------------------------------------------
// RREQ
//-----------------------------------------------------------------------------
RreqHeader::RreqHeader () : rq_flags(0), reserved(0), rq_hop_count(0), rq_bcast_id(0), rq_dst_seqno(0), rq_src_seqno(0)
{
  // TODO check defaults in AODV UU
  SetGratiousRrep (false);
  SetDestinationOnly (false);
  SetUnknownSeqno (false);
}

uint32_t 
RreqHeader::GetSerializedSize ()
{
  return 24;
}

void 
RreqHeader::Serialize (Buffer::Iterator i)
{
  i.WriteU8 (type());
  i.WriteU8 (rq_flags);
  i.WriteU8 (reserved);
  i.WriteU8 (rq_hop_count);
  i.WriteHtonU32 (rq_bcast_id);
  WriteTo (i, rq_dst);
  i.WriteHtonU32 (rq_dst_seqno);
  WriteTo (i, rq_src);
  i.WriteHtonU32 (rq_src_seqno);
}

uint32_t 
RreqHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t t = i.ReadU8 ();
  NS_ASSERT (t == type());
  
  rq_flags = i.ReadU8 ();
  reserved = i.ReadU8 ();
  rq_hop_count = i.ReadU8 ();
  rq_bcast_id = i.ReadNtohU32 ();
  ReadFrom (i, rq_dst);
  rq_dst_seqno = i.ReadNtohU32 ();
  ReadFrom (i, rq_src);
  rq_src_seqno = i.ReadNtohU32 ();
  
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void 
RreqHeader::Print (std::ostream &os) const
{
  // TODO
}

std::ostream & operator<<(std::ostream & os, RreqHeader const & h)
{
  h.Print (os);
  return os;
}

void 
RreqHeader::SetGratiousRrep (bool f)
{
  if (f) rq_flags |= (1 << 2);
  else   rq_flags &= ~(1 << 2);
}

bool 
RreqHeader::GetGratiousRrep () const
{
  return (rq_flags & (1 << 2));
}

void 
RreqHeader::SetDestinationOnly (bool f)
{
  if (f) rq_flags |= (1 << 3);
  else   rq_flags &= ~(1 << 3);
}

bool 
RreqHeader::GetDestinationOnly () const
{
  return (rq_flags & (1 << 3));
}

void 
RreqHeader::SetUnknownSeqno (bool f)
{
  if (f) rq_flags |= (1 << 4);
  else   rq_flags &= ~(1 << 4);
}

bool 
RreqHeader::GetUnknownSeqno () const
{
  return (rq_flags & (1 << 4));
}

bool
RreqHeader::operator==(RreqHeader const & o) const
{
  return (rq_flags == o.rq_flags && reserved == o.reserved &&
      rq_hop_count == o.rq_hop_count && rq_bcast_id == o.rq_bcast_id &&
      rq_dst == o.rq_dst && rq_dst_seqno == o.rq_dst_seqno && 
      rq_src == o.rq_src && rq_src_seqno == o.rq_src_seqno);
}

}}