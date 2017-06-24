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
#include <iostream>

namespace nfd {

namespace face {

namespace bluetooth = boost::asio::bluetooth;

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


  // if (!configSection) {
  //   if (!context.isDryRun && !m_channels.empty()) {
  //     NFD_LOG_WARN("Cannot disable tcp4 and tcp6 channels after initialization");
  //   }
  //   return;
  // }

  bool wantListen = true;
  // default channel value
  uint8_t channel = 1;

  // load config file for Bluetooth
  for (const auto& pair : *configSection) {
    const std::string& key = pair.first;

    if (key == "listen") {
      wantListen = ConfigFile::parseYesNo(pair, "face_system.bluetooth");
    }
    else if (key == "channel") {
      channel = ConfigFile::parseNumber<uint16_t>(pair, "face_system.bluetooth");
    }
    else {
      BOOST_THROW_EXCEPTION(ConfigFile::Error("Unrecognized option face_system.bluetooth." + key));
    }
  }

  // if not dryrun
  if (!context.isDryRun) {
    providedSchemes.insert("bluetooth");

    // create a local endpoint with specified channel
    boost::asio::bluetooth::bluetooth::endpoint endpoint(channel);
    // create a bt channel using the endpoint
    shared_ptr<BluetoothChannel> btChannel = this->createChannel(endpoint);

    if (wantListen && !btChannel->isListening()) {
      //
      btChannel->listen(context.addFace, nullptr);
    }
  }

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
  // std::cout << "@@ createFace - uri.getMac(): " << uri.getMac() << " uri.getChannel(): " << uri.getChannel() << std::endl;

  boost::asio::bluetooth::bluetooth::endpoint endpoint(uri.getMac(),
                               (uint8_t)boost::lexical_cast<int>(uri.getChannel()));

  for (const auto& i : m_channels) {
    i.second->connect(endpoint, wantLocalFieldsEnabled, onCreated, onFailure);
    return;
  }

  NFD_LOG_TRACE("No channels available to connect to " + boost::lexical_cast<std::string>(endpoint));
  onFailure(504, "No channels available to connect");
}

shared_ptr<BluetoothChannel>
BluetoothFactory::createChannel(const boost::asio::bluetooth::bluetooth::endpoint& endpoint)
{
  // If the channel exists, don't create a new one but return it
  auto channel = findChannel(endpoint);
  if (channel) {
    return channel;
  }

  // Create a new channel
  channel = make_shared<BluetoothChannel>(endpoint);
  m_channels[endpoint] = channel;

  NFD_LOG_DEBUG("Channel [" << endpoint << "] created");
  return channel;
}

shared_ptr<BluetoothChannel>
BluetoothFactory::createChannel(const std::string& localMac, const std::string& localChannel)
{
  boost::asio::bluetooth::bluetooth::endpoint endpoint(localMac, (uint8_t)boost::lexical_cast<int>(localChannel));
  return createChannel(endpoint);
}

std::vector<shared_ptr<const Channel>>
BluetoothFactory::getChannels() const
{
  return getChannelsFromMap(m_channels);
}

shared_ptr<BluetoothChannel>
BluetoothFactory::findChannel(const boost::asio::bluetooth::bluetooth::endpoint& localEndpoint) const
{

  auto i = m_channels.find(localEndpoint);
  if (i != m_channels.end())
    return i->second;
  else
    return nullptr;
}

} // namespace face
} // namespace nfd
