#include "bluetooth-channel.hpp"
#include "generic-link-service.hpp"
#include "bluetooth-transport.hpp"
#include "core/global-io.hpp"
#include <iostream>

namespace nfd {

NFD_LOG_INIT("BluetoothChannel");

//namespace bluetooth = boost::asio::bluetooth;

BluetoothChannel::BluetoothChannel(const boost::asio::bluetooth::bluetooth::endpoint& localEndpoint)
  : m_localEndpoint(localEndpoint)
  , m_acceptor(getGlobalIoService())
  , m_acceptSocket(getGlobalIoService())

{
  // setUri
  setUri(FaceUri(m_localEndpoint));
}

void
BluetoothChannel::listen(const FaceCreatedCallback& onFaceCreated,
                         const FaceCreationFailedCallback& onAcceptFailed,
                         int backlog/* = bluetooth::acceptor::max_connections*/)
{
  // if already listening
  if (isListening()) {
    NFD_LOG_WARN("[" << m_localEndpoint << "] Already listening");
    return;
  }

  m_acceptor.open(m_localEndpoint.protocol());
  // m_acceptor.set_option(boost::asio::bluetooth::bluetooth::acceptor::reuse_address(true));

  m_acceptor.bind(m_localEndpoint);
  m_acceptor.listen(backlog);

  // start accepting connections
  accept(onFaceCreated, onAcceptFailed);
}

void
BluetoothChannel::connect(const boost::asio::bluetooth::bluetooth::endpoint& remoteEndpoint,
                    bool wantLocalFieldsEnabled,
                    const FaceCreatedCallback& onFaceCreated,
                    const FaceCreationFailedCallback& onConnectFailed,
                    const time::seconds& timeout/* = time::seconds(4)*/)
{
  auto it = m_channelFaces.find(remoteEndpoint);
  if (it != m_channelFaces.end()) {
    onFaceCreated(it->second);
    return;
  }

  auto clientSocket = make_shared<boost::asio::bluetooth::bluetooth::socket>(ref(getGlobalIoService()));

  scheduler::EventId connectTimeoutEvent = scheduler::schedule(timeout,
    bind(&BluetoothChannel::handleConnectTimeout, this, clientSocket, onConnectFailed));

  std::cout << "remoteEndpoint.address(): " << remoteEndpoint.address() << " channel: " << (int)remoteEndpoint.channel() << std::endl;

  clientSocket->async_connect(remoteEndpoint,
                              bind(&BluetoothChannel::handleConnect, this,
                                   boost::asio::placeholders::error, clientSocket,
                                   wantLocalFieldsEnabled, connectTimeoutEvent,
                                   onFaceCreated, onConnectFailed));
}

size_t
BluetoothChannel::size() const
{
  return m_channelFaces.size();
}

void
BluetoothChannel::createFace(boost::asio::bluetooth::bluetooth::socket&& socket,
                       bool isOnDemand,
                       bool wantLocalFieldsEnabled,
                       const FaceCreatedCallback& onFaceCreated)
{
  shared_ptr<Face> face;
  boost::asio::bluetooth::bluetooth::endpoint remoteEndpoint = socket.remote_endpoint();
  std::cout << "remote endpoint: " << socket.remote_endpoint().address() << std::endl;
  std::cout << "local endpoint: " << socket.local_endpoint().address() << std::endl;

  auto it = m_channelFaces.find(remoteEndpoint);
  if (it == m_channelFaces.end()) {
    auto persistency = isOnDemand ? ndn::nfd::FACE_PERSISTENCY_ON_DEMAND
                                  : ndn::nfd::FACE_PERSISTENCY_PERSISTENT;


    auto linkService = make_unique<face::GenericLinkService>();
    auto options = linkService->getOptions();
    options.allowLocalFields = wantLocalFieldsEnabled;
    linkService->setOptions(options);

    auto transport = make_unique<face::BluetoothTransport>(std::move(socket), persistency);

    face = make_shared<Face>(std::move(linkService), std::move(transport));

    m_channelFaces[remoteEndpoint] = face;
    connectFaceClosedSignal(*face,
      [this, remoteEndpoint] {
        NFD_LOG_TRACE("Erasing " << remoteEndpoint << " from channel face map");
        m_channelFaces.erase(remoteEndpoint);
      });
  }
  else {
    // we already have a face for this endpoint, just reuse it
    face = it->second;

    boost::system::error_code error;
    // @@ can both be shutdown
    socket.shutdown(boost::asio::bluetooth::bluetooth::socket::shutdown_both, error);
    socket.close(error);
  }

  // Need to invoke the callback regardless of whether or not we have already created
  // the face so that control responses and such can be sent.
  onFaceCreated(face);
}

void
BluetoothChannel::accept(const FaceCreatedCallback& onFaceCreated,
                         const FaceCreationFailedCallback& onAcceptFailed)
{
  m_acceptor.async_accept(m_acceptSocket, bind(&BluetoothChannel::handleAccept, this,
                                               boost::asio::placeholders::error,
                                               onFaceCreated, onAcceptFailed));
}

void
BluetoothChannel::handleAccept(const boost::system::error_code& error,
                         const FaceCreatedCallback& onFaceCreated,
                         const FaceCreationFailedCallback& onAcceptFailed)
{
  if (error) {
    if (error == boost::asio::error::operation_aborted) // when the socket is closed by someone
      return;

    NFD_LOG_DEBUG("[" << m_localEndpoint << "] Accept failed: " << error.message());
    if (onAcceptFailed)
      onAcceptFailed(500, "Accept failed: " + error.message());
    return;
  }

  NFD_LOG_DEBUG("[" << m_localEndpoint << "] Connection from " << m_acceptSocket.remote_endpoint());

  createFace(std::move(m_acceptSocket), true, false, onFaceCreated);

  // prepare accepting the next connection
  accept(onFaceCreated, onAcceptFailed);
}


void
BluetoothChannel::handleConnect(const boost::system::error_code& error,
                                const shared_ptr<boost::asio::bluetooth::bluetooth::socket>& socket,
                                bool wantLocalFieldsEnabled,
                                const scheduler::EventId& connectTimeoutEvent,
                                const FaceCreatedCallback& onFaceCreated,
                                const FaceCreationFailedCallback& onConnectFailed)
{
  scheduler::cancel(connectTimeoutEvent);

#if (BOOST_VERSION == 105400)
  // To handle regression in Boost 1.54
  // https://svn.boost.org/trac/boost/ticket/8795
  boost::system::error_code anotherErrorCode;
  socket->remote_endpoint(anotherErrorCode);
  if (error || anotherErrorCode) {
#else
  if (error) {
#endif
    if (error == boost::asio::error::operation_aborted) // when the socket is closed by someone
      return;

    NFD_LOG_WARN("[" << m_localEndpoint << "] Connect failed: " << error.message());

    socket->close();

    if (onConnectFailed)
      onConnectFailed(504, "Connect failed: " + error.message());
    return;
  }

  NFD_LOG_DEBUG("[" << m_localEndpoint << "] Connected to " << socket->remote_endpoint());

  createFace(std::move(*socket), false, wantLocalFieldsEnabled, onFaceCreated);
}


void
BluetoothChannel::handleConnectTimeout(const shared_ptr<boost::asio::bluetooth::bluetooth::socket>& socket,
                                       const FaceCreationFailedCallback& onConnectFailed)
{
  NFD_LOG_DEBUG("Connect to remote endpoint timed out");

  // abort the connection attempt
  boost::system::error_code error;
  socket->close(error);

  if (onConnectFailed)
    onConnectFailed(504, "Connect to remote endpoint timed out");
}

} // namespace nfd
