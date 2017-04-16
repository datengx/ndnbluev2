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

#include "bluetooth-factory.hpp"
#include "core/logger.hpp"

namespace nfd {

namespace face {

NFD_LOG_INIT("BluetoothFactory");
// register factory to be visiable by the face system
NFD_REGISTER_PROTOCOL_FACTORY(BluetoothFactory);

const std::string&
BluetoothFactory::getId()
{
  static std::string id("bluetooth");
  return id;
}


void
BluetoothFactory::processConfig(OptionalConfigSection configSection,
                          FaceSystem::ConfigContext& context)
{
  // tcp
  // {
  //   listen yes
  //   port 6363
  //   enable_v4 yes
  //   enable_v6 yes
  // }


  // if (!configSection) {
  //   if (!context.isDryRun && !m_channels.empty()) {
  //     NFD_LOG_WARN("Cannot disable tcp4 and tcp6 channels after initialization");
  //   }
  //   return;
  // }

}


void
BluetoothFactory::createFace(const FaceUri& uri,
                       ndn::nfd::FacePersistency persistency,
                       bool wantLocalFieldsEnabled,
                       const FaceCreatedCallback& onCreated,
                       const FaceCreationFailedCallback& onFailure)
{
  BOOST_ASSERT(uri.isCanonical());

  if (persistency != ndn::nfd::FACE_PERSISTENCY_PERSISTENT) {
    NFD_LOG_TRACE("createFace only supports FACE_PERSISTENCY_PERSISTENT");
    onFailure(406, "Outgoing Bluetooth faces only support persistent persistency");
    return;
  }

}

std::vector<shared_ptr<const Channel>>
BluetoothFactory::getChannels() const
{

}

} // namespace face
} // namespace nfd
