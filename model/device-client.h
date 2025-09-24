#ifndef DEVICE_CLIENT_H
#define DEVICE_CLIENT_H

#include "ns3/application.h"
#include "ns3/callback.h"
#include "ns3/event-id.h"
// #include "ns3/ipv4-address.h"
#include "ns3/payload.h"
#include "ns3/processing-time.h"
#include "ns3/ptr.h"
#include "ns3/seq-ts-header.h"
#include "ns3/traced-callback.h"

#include <map>
#include <string>

namespace ns3 {

class Socket;
class Packet;

class DeviceClient : public Application 
{
public:
  typedef Callback<Payload, uint64_t, int64_t> MessageSendCallbackType;
  typedef Callback<void, std::string, uint32_t, bool, uint64_t, int64_t> MessageReceiveCallbackType;

  static TypeId GetTypeId (void);
  DeviceClient();
  virtual ~DeviceClient ();

  uint64_t GetFromNodeId();
  int64_t GetToNodeId();
  uint32_t GetSent();
  std::vector<int64_t> GetSendRequestTimestamps();
  std::vector<int64_t> GetReplyTimestamps();
  std::vector<int64_t> GetReceiveReplyTimestamps();

  static Payload defaultSendCallbackImpl(uint64_t from, int64_t to) { return Payload(0); }
  static void defaultReceiveCallbackImpl(std::string str, uint32_t payloadId, bool isReply, uint64_t from, int64_t to) {}

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void ScheduleProcessing (Time dt);
  void Process (void);
  void Send (Ptr<Packet> p);
  void HandleRead (Ptr<Socket> socket);

  Time m_interval; //!< Packet inter-send time
  Ptr<Socket> m_socket; //!< Socket

  bool m_sendData;
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port

  EventId m_processEvent; //!< Event to process the next packet
  EventId m_sendEvent; //!< Event to send the next packet

  uint64_t m_fromNodeId;
  int64_t m_toNodeId; //!< Set to negative values if unused.
  uint32_t m_sent; //!< Counter for sent packets
  std::map<uint32_t, uint32_t> m_sentPayloadIds;
  std::vector<int64_t> m_sendRequestTimestamps;
  std::vector<int64_t> m_replyTimestamps;
  std::vector<int64_t> m_receiveReplyTimestamps;

  Time m_processingTimeConstant; //!< Constant term of processing time.
  Time m_processingTimeMean; //!< Average of stochastic term of processing time.
  Time m_processingTimeStdDev; //!< Standard deviation of stochastic term of processing time.
  Ptr<ProcessingTime> m_processingTime;

  MessageSendCallbackType m_msgSendCallback;
  MessageReceiveCallbackType m_msgReceiveCallback;
};

} // namespace ns3

#endif /* DEVICE_CLIENT_H */
