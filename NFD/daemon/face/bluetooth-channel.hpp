#ifndef NFD_DAEMON_FACE_BLUETOOTH_CHANNEL_HPP
#define NFD_DAEMON_FACE_BLUETOOTH_CHANNEL_HPP

#include "channel.hpp"
#include "core/scheduler.hpp"

namespace nfd {
namespace bluetooth {
typedef boost::asio::bluetooth::bluetooth::endpoint Endpoint;
} // namespace bluetooth

class BluetoothChannel : public Channel
{
public:


  /** \brief Create Bluetooth Channel for the local endpoint
   */
  explicit
  BluetoothChannel(const bluetooth::Endpoint& localEndpoint);

  /** \brief Enable listening on the local endpoint, accept connections,
   *         and create faces when remote host makes a connection
   *  \param onFaceCreated  Callback to notify successful creation of the face
   *  \param onAcceptFailed Callback to notify when channel fails (accept call
   *                        returns an error)
   *  \param backlog        The maximum length of the queue of pending incoming
   *                        connections
   *
   */
  void
  listen(const FaceCreatedCallback& onFaceCreated,
         const FaceCreationFailedCallback& onAcceptFailed,
         int backlog = boost::asio::bluetooth::bluetooth::acceptor::max_connections);


  /**
   * \brief Create a face by establishing connection to remote endpoint
   */
  void
  connect(const bluetooth::Endpoint& remoteEndpoint,
          bool wantLocalFieldsEnabled,
          const FaceCreatedCallback& onFaceCreated,
          const FaceCreationFailedCallback& onConnectFailed,
          const time::seconds& timeout = time::seconds(4));

  /**
   * \brief Get number of faces in the channel
   */
  size_t
  size() const;

  bool
  isListening() const;

private:
  void
  createFace(boost::asio::bluetooth::bluetooth::socket&& socket,
             bool isOnDemand,
             bool wantLocalFieldsEnabled,
             const FaceCreatedCallback& onFaceCreated);

  void
  accept(const FaceCreatedCallback& onFaceCreated,
         const FaceCreationFailedCallback& onAcceptFailed);

  void
  handleAccept(const boost::system::error_code& error,
               const FaceCreatedCallback& onFaceCreated,
               const FaceCreationFailedCallback& onAcceptFailed);

  void
  handleConnect(const boost::system::error_code& error,
                const shared_ptr<boost::asio::bluetooth::bluetooth::socket>& socket,
                bool wantLocalFieldsEnabled,
                const scheduler::EventId& connectTimeoutEvent,
                const FaceCreatedCallback& onFaceCreated,
                const FaceCreationFailedCallback& onConnectFailed);

  void
  handleConnectTimeout(const shared_ptr<boost::asio::bluetooth::bluetooth::socket>& socket,
                       const FaceCreationFailedCallback& onConnectFailed);


private:
  std::map<bluetooth::Endpoint, shared_ptr<Face>> m_channelFaces;

  bluetooth::Endpoint m_localEndpoint;
  boost::asio::bluetooth::bluetooth::acceptor m_acceptor;
  boost::asio::bluetooth::bluetooth::socket m_acceptSocket;
};

inline bool
BluetoothChannel::isListening() const
{
  return m_acceptor.is_open();
}

} // namespace nfd


#endif // NFD_DAEMON_FACE_BLUETOOTH_FACTORY_HPP
