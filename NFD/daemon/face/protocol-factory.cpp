/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  Regents of the University of California,
 *                           Arizona Board of Regents,
 *                           Colorado State University,
 *                           University Pierre & Marie Curie, Sorbonne University,
 *                           Washington University in St. Louis,
 *                           Beijing Institute of Technology,
 *                           The University of Memphis.
 *
 * This file is part of NFD (Named Data Networking Forwarding Daemon).
 * See AUTHORS.md for complete list of NFD authors and contributors.
 *
 * NFD is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NFD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NFD, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "protocol-factory.hpp"
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

namespace nfd {
namespace face {

ProtocolFactory::Registry&
ProtocolFactory::getRegistry()
{
  static Registry registry;
  return registry;
}

unique_ptr<ProtocolFactory>
ProtocolFactory::create(const std::string& id)
{
  Registry& registry = getRegistry();
  auto found = registry.find(id);
  return found == registry.end() ? nullptr : found->second();
}

std::set<std::string>
ProtocolFactory::listRegistered()
{
  std::set<std::string> factoryIds;
  // Obtain all the keys of the map return by getRegistry
  // And copy them to a set of string
  boost::copy(getRegistry() | boost::adaptors::map_keys,
              std::inserter(factoryIds, factoryIds.end()));
  return factoryIds;
}

} // namespace face
} // namespace nfd
