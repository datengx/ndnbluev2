#include "bluetooth-transport.hpp"


namespace nfd {
namespace face {

NFD_LOG_INCLASS_TEMPLATE_SPECIALIZATION_DEFINE(StreamTransport, BluetoothTransport::protocol, "BluetoothTransport");

time::milliseconds BluetoothTransport::s_initialReconnectWait = time::seconds(1);
time::milliseconds BluetoothTransport::s_maxReconnectWait = time::minutes(5);
// exponential backoff
float BluetoothTransport::s_reconnectWaitMultiplier = 2.0f;


// Socket pair has been setup, is passed in to the constructor
BluetoothTransport::BluetoothTransport(protocol::socket&& socket, ndn::nfd::FacePersistency persistency)
  : StreamTransport(std::move(socket))
  , m_remoteEndpoint(m_socket.remote_endpoint())
  , m_nextReconnectWait(s_initialReconnectWait)
{
  this->setLocalUri(FaceUri(m_socket.local_endpoint()));
  this->setRemoteUri(FaceUri(m_socket.remote_endpoint()));

  this->setPersistency(persistency);
  this->setLinkType(ndn::nfd::LINK_TYPE_POINT_TO_POINT);
  this->setMtu(MTU_UNLIMITED);

  NFD_LOG_FACE_INFO("Creating transport");
}

bool
BluetoothTransport::canChangePersistencyToImpl(ndn::nfd::FacePersistency newPersistency) const
{
  return true;
}

void
BluetoothTransport::afterChangePersistency(ndn::nfd::FacePersistency oldPersistency)
{
  // if persistency was changed from permanent to any other value
  if (oldPersistency == ndn::nfd::FACE_PERSISTENCY_PERMANENT) {
    if (this->getState() == TransportState::DOWN) {
      // non-permanent transport cannot be in DOWN state, so fail hard
      this->setState(TransportState::FAILED);
      doClose();
    }
  }
}

void
BluetoothTransport::handleError(const boost::system::error_code& error)
{
  if (this->getPersistency() == ndn::nfd::FACE_PERSISTENCY_PERMANENT) {
    NFD_LOG_FACE_TRACE("Bluetooth socket error: " << error.message());
    this->setState(TransportState::DOWN);

    // cancel all outstanding operations
    boost::system::error_code error;
    m_socket.cancel(error);

    // do this asynchronously because there could be some callbacks still pending
    getGlobalIoService().post([this] { reconnect(); });
  }
  else {
    StreamTransport::handleError(error);
  }
}

void
BluetoothTransport::reconnect()
{
  NFD_LOG_FACE_TRACE(__func__);

  if (getState() == TransportState::CLOSING ||
      getState() == TransportState::FAILED ||
      getState() == TransportState::CLOSED) {
    // transport is shutting down, don't attempt to reconnect
    return;
  }

  BOOST_ASSERT(getPersistency() == ndn::nfd::FACE_PERSISTENCY_PERMANENT);
  BOOST_ASSERT(getState() == TransportState::DOWN);

  // recreate the socket
  m_socket = protocol::socket(m_socket.get_io_service());
  this->resetReceiveBuffer();
  this->resetSendQueue();

  m_reconnectEvent = scheduler::schedule(m_nextReconnectWait,
                                         [this] { handleReconnectTimeout(); });
  m_socket.async_connect(m_remoteEndpoint,
                         [this] (const boost::system::error_code& error) { handleReconnect(error); });
}

void
BluetoothTransport::handleReconnect(const boost::system::error_code& error)
{
  // The transport is shutting thus no reconect is permitted,
  // immediately return
  if (getState() == TransportState::CLOSING ||
      getState() == TransportState::FAILED ||
      getState() == TransportState::CLOSED ||
      error == boost::asio::error::operation_aborted) {
    // transport is shutting down, abort the reconnection attempt and ignore any errors
    return;
  }

  //
  if (error) {
    NFD_LOG_FACE_TRACE("Reconnection attempt failed: " << error.message());
    return;
  }

  m_reconnectEvent.cancel();
  m_nextReconnectWait = s_initialReconnectWait;

  this->setLocalUri(FaceUri(m_socket.local_endpoint()));
  NFD_LOG_FACE_TRACE("Bluetooth connection reestablished");
  this->setState(TransportState::UP);
  this->startReceive();
}

void
BluetoothTransport::handleReconnectTimeout()
{
  // abort the reconnection attempt
  boost::system::error_code error;
  m_socket.close(error);

  // exponentially back off the reconnection timer
  m_nextReconnectWait =
      std::min(time::duration_cast<time::milliseconds>(m_nextReconnectWait * s_reconnectWaitMultiplier),
               s_maxReconnectWait);

  // do this asynchronously because there could be some callbacks still pending
  getGlobalIoService().post([this] { reconnect(); });
}

void
BluetoothTransport::doClose()
{
  m_reconnectEvent.cancel();
  StreamTransport::doClose();
}

} // namespace face
} // namespace nfd
