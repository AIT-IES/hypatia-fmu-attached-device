#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "device-client.h"

using namespace std;

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DeviceClient");

NS_OBJECT_ENSURE_REGISTERED (DeviceClient);

TypeId
DeviceClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::DeviceClient")
        .SetParent<Application>()
        .SetGroupName("Applications")
        .AddConstructor<DeviceClient>()
        .AddAttribute("SendData",
                      "Flag to indicate if data should be sent.",
                      BooleanValue(true),
                      MakeBooleanAccessor(&DeviceClient::m_sendData),
                      MakeBooleanChecker())
        .AddAttribute("Interval",
                      "The time to wait between packets",
                      TimeValue(Seconds(1.0)),
                      MakeTimeAccessor(&DeviceClient::m_interval),
                      MakeTimeChecker())
        .AddAttribute("RemoteAddress",
                      "The destination Address of the outbound packets",
                      AddressValue(),
                      MakeAddressAccessor(&DeviceClient::m_peerAddress),
                      MakeAddressChecker())
        .AddAttribute("RemotePort",
                      "The destination port of the outbound packets",
                      UintegerValue(0),
                      MakeUintegerAccessor(&DeviceClient::m_peerPort),
                      MakeUintegerChecker<uint16_t>())
        .AddAttribute("FromNodeId",
                      "From node identifier",
                      UintegerValue(0),
                      MakeUintegerAccessor(&DeviceClient::m_fromNodeId),
                      MakeUintegerChecker<uint64_t>())
        .AddAttribute("ToNodeId",
                      "To node identifier",
                      IntegerValue(-1),
                      MakeIntegerAccessor(&DeviceClient::m_toNodeId),
                      MakeIntegerChecker<int64_t>())
        .AddAttribute("MsgSendCallback",
                      "Callback for sending a payload message",
                      CallbackValue(MakeCallback(&DeviceClient::defaultSendCallbackImpl)),
                      MakeCallbackAccessor(&DeviceClient::m_msgSendCallback),
                      MakeCallbackChecker())
        .AddAttribute("MsgReceiveCallback",
                      "Callback for receiving a payload message",
                      CallbackValue(MakeCallback(&DeviceClient::defaultReceiveCallbackImpl)),
                      MakeCallbackAccessor(&DeviceClient::m_msgReceiveCallback),
                      MakeCallbackChecker())
        .AddAttribute("ProcessingTimeConstant",
                      "Constant term of processing time",
                      TimeValue(Seconds(0)),
                      MakeTimeAccessor(&DeviceClient::m_processingTimeConstant),
                      MakeTimeChecker())
        .AddAttribute("ProcessingTimeMean",
                      "Average of stochastic term of processing time",
                      TimeValue(MilliSeconds(1)),
                      MakeTimeAccessor(&DeviceClient::m_processingTimeMean),
                      MakeTimeChecker())
        .AddAttribute("ProcessingTimeStdDev",
                      "Standard deviation of stochastic term of processing time",
                      TimeValue(MicroSeconds(50)),
                      MakeTimeAccessor(&DeviceClient::m_processingTimeStdDev),
                      MakeTimeChecker());
    return tid;
}

DeviceClient::DeviceClient() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_sent = 0;
    m_processEvent = EventId();
    m_sendEvent = EventId();
    m_msgSendCallback = MakeCallback(&DeviceClient::defaultSendCallbackImpl);
    m_msgReceiveCallback = MakeCallback(&DeviceClient::defaultReceiveCallbackImpl);
    m_processingTime = 0;
}

DeviceClient::~DeviceClient() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
}

void
DeviceClient::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
DeviceClient::StartApplication(void) {
    NS_LOG_FUNCTION(this);
    static uint16_t port = 1025; 
    if (m_socket == 0)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);

        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port++);
        if (m_socket->Bind(local) == -1) {
            NS_FATAL_ERROR("Failed to bind socket");
        }

        if (m_sendData)
        { 
            if (Ipv4Address::IsMatchingType(m_peerAddress) == true) {
                m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
            } else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
                m_socket->Connect(m_peerAddress);
            } else {
                NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
            }

            ScheduleProcessing(Seconds(0.)); 
        }
    }
    m_socket->SetRecvCallback(MakeCallback(&DeviceClient::HandleRead, this));
    m_socket->SetAllowBroadcast(true);

    m_processingTime = CreateObject<ProcessingTime>(m_processingTimeConstant, m_processingTimeMean, m_processingTimeStdDev);
}

void
DeviceClient::StopApplication() {
    NS_LOG_FUNCTION(this);
    if (m_socket != 0) {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        m_socket = 0;
    }
    Simulator::Cancel(m_sendEvent);
}

void
DeviceClient::ScheduleProcessing(Time dt) {
    NS_LOG_FUNCTION(this << dt);
    m_processEvent = Simulator::Schedule(dt, &DeviceClient::Process, this);
}

void
DeviceClient::Process(void) {
    NS_LOG_FUNCTION(this << " - start processing at " << Simulator::Now());
    NS_ABORT_MSG_UNLESS(m_processEvent.IsExpired(), "Previous processing has not finished yet.");
    NS_ABORT_MSG_UNLESS(m_sendEvent.IsExpired(), "Previous message has not been sent yet.");

    // Packet with message and timestamp.
    const Payload& pl = m_msgSendCallback(m_fromNodeId, m_toNodeId);
    Ptr<Packet> p = pl.GetTransmitBuffer() ? 
        Create<Packet>((uint8_t*) pl.GetBuffer().c_str(), pl.GetBufferSize()) :
        Create<Packet>(pl.GetBufferSize());

    // Creates one with the current timestamp
    SeqTsHeader seqTs;
    seqTs.SetSeq(pl.GetId());
    p->AddHeader(seqTs);

    // Timestamps
    m_sentPayloadIds[pl.GetId()] = m_sent;
    m_sendRequestTimestamps.push_back(Simulator::Now().GetNanoSeconds());
    m_replyTimestamps.push_back(-1);
    m_receiveReplyTimestamps.push_back(-1);
    m_sent++;

    // Send out
    m_sendEvent = Simulator::Schedule(
        m_processingTime->GetValue(), &DeviceClient::Send, this, p
    );

    // Schedule next transmit
    ScheduleProcessing(m_interval);
}

void
DeviceClient::Send (Ptr<Packet> p) {
    NS_LOG_FUNCTION(this << " - send packet at " << Simulator::Now());
    m_socket->Send(p);
}

void
DeviceClient::HandleRead(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    Ptr <Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from))) {

        // Receiving header
        SeqTsHeader incomingSeqTs;
        packet->RemoveHeader (incomingSeqTs);
        uint32_t payloadId = incomingSeqTs.GetSeq();

        map<uint32_t, uint32_t>::iterator itFindSent = m_sentPayloadIds.find(payloadId);
        bool isReply = (itFindSent != m_sentPayloadIds.end());

        if (isReply) {
            uint32_t sent = itFindSent->second;

            // Update the local timestamps
            m_replyTimestamps[sent] = incomingSeqTs.GetTs().GetNanoSeconds();
            m_receiveReplyTimestamps[sent] = Simulator::Now().GetNanoSeconds();

            m_sentPayloadIds.erase(itFindSent);
        }

        uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData(buffer, packet->GetSize ());
        string s = string(buffer, buffer+packet->GetSize());
        m_msgReceiveCallback(s, payloadId, isReply, m_fromNodeId, (isReply ? m_toNodeId : -1));
        NS_LOG_DEBUG ("Buffer: size = " << packet->GetSize() << " - content = >>" << s << "<<");
        delete buffer;
    }
}

uint64_t DeviceClient::GetFromNodeId() {
    return m_fromNodeId;
}

int64_t DeviceClient::GetToNodeId() {
    return m_toNodeId;
}

uint32_t DeviceClient::GetSent() {
    return m_sent;
}

std::vector<int64_t> DeviceClient::GetSendRequestTimestamps() {
    return m_sendRequestTimestamps;
}

std::vector<int64_t> DeviceClient::GetReplyTimestamps() {
    return m_replyTimestamps;
}

std::vector<int64_t> DeviceClient::GetReceiveReplyTimestamps() {
    return m_receiveReplyTimestamps;
}

} // Namespace ns3
