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

#include "face-system.hpp"
#include "protocol-factory.hpp"
#include "core/logger.hpp"
#include "fw/face-table.hpp"
#include <iostream>

namespace nfd {
namespace face {

NFD_LOG_INIT("FaceSystem");

FaceSystem::FaceSystem(FaceTable& faceTable)
  : m_faceTable(faceTable)
{
  for (const std::string& id : ProtocolFactory::listRegistered()) {
    std::cout << "creating factory " << id << std::endl;
    m_factories[id] = ProtocolFactory::create(id);
  }
}

FaceSystem::~FaceSystem() = default;

std::set<const ProtocolFactory*>
FaceSystem::listProtocolFactories() const
{
  std::set<const ProtocolFactory*> factories;
  for (const auto& p : m_factories) {
    factories.insert(p.second.get());
  }
  return factories;
}

ProtocolFactory*
FaceSystem::getFactoryById(const std::string& id)
{
  auto found = m_factories.find(id);
  return found == m_factories.end() ? nullptr : found->second.get();
}

ProtocolFactory*
FaceSystem::getFactoryByScheme(const std::string& scheme)
{
  auto found = m_factoryByScheme.find(scheme);
  return found == m_factoryByScheme.end() ? nullptr : found->second;
}

void
FaceSystem::setConfigFile(ConfigFile& configFile)
{
  configFile.addSectionHandler("face_system", bind(&FaceSystem::processConfig, this, _1, _2, _3));
}

void
FaceSystem::processConfig(const ConfigSection& configSection, bool isDryRun, const std::string& filename)
{
  ConfigContext context;
  context.isDryRun = isDryRun;
  context.addFace = bind(&FaceTable::add, &m_faceTable, _1);
  context.m_netifs = listNetworkInterfaces();

  // process sections in protocol factories
  for (const auto& pair : m_factories) {
    // @@
    if (pair.first == "bluetooth") {
        // uncomment the following line to skip for now
        //continue;
    }

    const std::string& sectionName = pair.first;
    // std::cout << "@@@@ m_factories[..].keys: " << sectionName << std::endl;
    ProtocolFactory* factory = pair.second.get();

    std::set<std::string> oldProvidedSchemes = factory->getProvidedSchemes();
    //@@
    // The tag in the configuration file is only optional.
    // If the returned is empty, then there is not such node with the
    // corresponding key.
    factory->processConfig(configSection.get_child_optional(sectionName), context);

    if (!isDryRun) {
      for (const std::string& scheme : factory->getProvidedSchemes()) {
        m_factoryByScheme[scheme] = factory;
        if (oldProvidedSchemes.erase(scheme) == 0) {
          NFD_LOG_TRACE("factory " << sectionName <<
                        " provides " << scheme << " FaceUri scheme");
        }
      }
      for (const std::string& scheme : oldProvidedSchemes) {
        m_factoryByScheme.erase(scheme);
        NFD_LOG_TRACE("factory " << sectionName <<
                      " no longer provides " << scheme << " FaceUri scheme");
      }
    }
  }

  // process other sections
  std::set<std::string> seenSections;
  for (const auto& pair : configSection) {
    const std::string& sectionName = pair.first;
    // const ConfigSection& subSection = pair.second;

    if (!seenSections.insert(sectionName).second) {
      BOOST_THROW_EXCEPTION(ConfigFile::Error("Duplicate section face_system." + sectionName));
    }

    if (m_factories.count(sectionName) > 0) {
      continue;
    }

    ///\todo #3521 nicfaces

    BOOST_THROW_EXCEPTION(ConfigFile::Error("Unrecognized option face_system." + sectionName));
  }
}

} // namespace face
} // namespace nfd
