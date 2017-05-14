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

#ifndef NFD_DAEMON_FACE_BLUETOOTH_FACTORY_HPP
#define NFD_DAEMON_FACE_BLUETOOTH_FACTORY_HPP

#include "protocol-factory.hpp"
#include "bluetooth-channel.hpp"

namespace nfd {

namespace face {

/** \brief protocol factory for Bluetooth over RFCOMM
 */
class BluetoothFactory : public ProtocolFactory
{
public:
  static const std::string&
  getId();

  /** \brief process face_system.bluetooth config section
   */
  void
  processConfig(OptionalConfigSection configSection,
                FaceSystem::ConfigContext& context) override;

  void
  createFace(const FaceUri& uri,
             ndn::nfd::FacePersistency persistency,
             bool wantLocalFieldsEnabled,
             const FaceCreatedCallback& onCreated,
             const FaceCreationFailedCallback& onFailure) override;


  shared_ptr<BluetoothChannel>
  createChannel(const boost::asio::bluetooth::bluetooth::endpoint& localEndpoint);

  // Create using MAC address and channel number
  shared_ptr<BluetoothChannel>
  createChannel(const std::string& localMac, const std::string& localChannel);

  std::vector<shared_ptr<const Channel>>
  getChannels() const override;

private:

private:
  shared_ptr<BluetoothChannel>
  findChannel(const boost::asio::bluetooth::bluetooth::endpoint& localEndpoint) const;

private:
  std::map<bluetooth::Endpoint, shared_ptr<BluetoothChannel>> m_channels;

};

} // namespace face
} // namespace nfd

#endif
