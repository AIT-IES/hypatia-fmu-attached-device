#ifndef DEVICE_CLIENT_H
#define DEVICE_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/seq-ts-header.h"
#include "ns3/callback.h"

#include <string>

namespace ns3 {

class Socket;
class Packet;

class DeviceClient : public Application 
{
public:
  typedef Callback<std::string, uint64_t, uint64_t> MessageSendCallbackType;
  typedef Callback<void, std::string, uint64_t, uint64_t> MessageReceiveCallbackType;

  static TypeId GetTypeId (void);
  DeviceClient();
  virtual ~DeviceClient ();

  uint64_t GetFromNodeId();
  uint64_t GetToNodeId();
  uint32_t GetSent();
  std::vector<int64_t> GetSendRequestTimestamps();
  std::vector<int64_t> GetReplyTimestamps();
  std::vector<int64_t> GetReceiveReplyTimestamps();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void ScheduleTransmit (Time dt);
  void Send (void);
  void HandleRead (Ptr<Socket> socket);

  static std::string defaultSendCallbackImpl(uint64_t from, uint64_t to) { return std::string(); }
  static void defaultReceiveCallbackImpl(std::string str, uint64_t from, uint64_t to) {}

  Time m_interval; //!< Packet inter-send time
  Ptr<Socket> m_socket; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet

  uint64_t m_fromNodeId;
  uint64_t m_toNodeId;
  uint32_t m_sent; //!< Counter for sent packets
  std::vector<int64_t> m_sendRequestTimestamps;
  std::vector<int64_t> m_replyTimestamps;
  std::vector<int64_t> m_receiveReplyTimestamps;

  MessageSendCallbackType m_msgSendCallback;
  MessageReceiveCallbackType m_msgReceiveCallback;
};

} // namespace ns3

#endif /* DEVICE_CLIENT_H */
