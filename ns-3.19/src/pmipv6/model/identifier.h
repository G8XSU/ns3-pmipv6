/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Proxy Mobile IPv6 (PMIPv6) (RFC5213) Implementation
 *
 * Copyright (c) 2010 KUT, ETRI
 * (Korea Univerity of Technology and Education)
 * (Electronics and Telecommunications Research Institute)
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
 * Author: Hyon-Young Choi <commani@gmail.com>
 */
 
#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <stdint.h>
#include <string.h>

#include <ostream>

#include "ns3/attribute-helper.h"

namespace ns3
{

class Mac48Address;

/**
 * \class Identifier
 * \brief Identifier.
 */
class Identifier
{
public:
  enum MaxSize_e {
    MAX_SIZE = 255
  };
  
  Identifier();
  Identifier(const uint8_t *buffer, uint8_t len);
  Identifier(const char *str);
  Identifier(Mac48Address addr);
  Identifier(const Identifier & identifier);
  Identifier &operator = (const Identifier &identifier);
  
  uint8_t GetLength (void) const;
  
  uint32_t CopyTo (uint8_t *buffer, uint8_t len) const;
  
  uint32_t CopyFrom (const uint8_t *buffer, uint8_t len);
  
  bool IsEmpty() const;

protected:

private:
  friend bool operator == (const Identifier &a, const Identifier &b);
  friend bool operator != (const Identifier &a, const Identifier &b);
  friend std::ostream& operator<< (std::ostream& os, const Identifier & identifier);
  
  uint8_t m_len;
  uint8_t m_identifier[MAX_SIZE];
};

ATTRIBUTE_HELPER_HEADER (Identifier);

bool operator == (const Identifier &a, const Identifier &b);
bool operator != (const Identifier &a, const Identifier &b);
std::ostream& operator<< (std::ostream& os, const Identifier & identifier);

/**
 * \class IdentifierHash
 * \brief Hash function class for Identifier.
 */
class IdentifierHash : public std::unary_function<Identifier, size_t>
{
public:
  /**
   * \brief Unary operator to hash Identifier.
   * \param x Identifier to hash
   */
  size_t operator () (Identifier const &x) const;
};

} /* namespace ns3 */

#endif /* MOBILITY_HEADER_H */
