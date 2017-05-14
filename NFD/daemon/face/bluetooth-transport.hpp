#ifndef NFD_DAEMON_FACE_BLUETOOTH_TRANSPORT_HPP
#define NFD_DAEMON_FACE_BLUETOOTH_TRANSPORT_HPP

#include "stream-transport.hpp"
#include "core/scheduler.hpp"

namespace nfd {
namespace face {

/**
 * \brief A Transport that communicates on a connected Bluetooth socket
 *
 * When persistency is set to permanent...
 */
class BluetoothTransport FINAL_UNLESS_WITH_TESTS : public StreamTransport<boost::asio::bluetooth::bluetooth>
{
public:
  BluetoothTransport(protocol::socket&& socket, ndn::nfd::FacePersistency persistency);

protected:
  bool
  canChangePersistencyToImpl(ndn::nfd::FacePersistency new Persistency) const final;

  void
  afterChangePersistency(ndn::nfd::FacePersistency oldPersistency) final;

  void
  doClose() final;

  void
  handleError(const boost::system::error_code& error) final;

PROTECTED_WITH_TESTS_ELSE_PRIVATE:
  VIRTUAL_WITH_TESTS void
  reconnect();

  VIRTUAL_WITH_TESTS void
  handleReconnect(const boost::system::error_code& error);

  VIRTUAL_WITH_TESTS void
  handleConnectTimeout();

PUBLIC_WITH_TESTS_ELSE_PRIVATE:

  static time::milliseconds s_initialReconnectWait;

  static time::milliseconds s_maxReconnectWait;

  static float s_reconnectWaitMultiplier;


private:
  typename protocol::endpoint m_remoteEndpoint;

  scheduler::ScopedEventId m_reconnectEvent;

  time::milliseconds m_nextReconnectWait;
};

} // namespace face
} // namespace nfd


#endif
