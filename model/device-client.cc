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
                          UintegerValue(0),
                          MakeUintegerAccessor(&DeviceClient::m_toNodeId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("MsgSendCallback",
                          "Callback for sending a payload message",
                          CallbackValue(MakeCallback(&DeviceClient::defaultSendCallbackImpl)),
                          MakeCallbackAccessor(&DeviceClient::m_msgSendCallback),
                          MakeCallbackChecker())
            .AddAttribute("MsgReceiveCallback",
                          "Callback for receiving a payload message",
                          CallbackValue(MakeCallback(&DeviceClient::defaultReceiveCallbackImpl)),
                          MakeCallbackAccessor(&DeviceClient::m_msgReceiveCallback),
                          MakeCallbackChecker());
    return tid;
}

DeviceClient::DeviceClient() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_sent = 0;
    m_sendEvent = EventId();
    m_msgSendCallback = MakeCallback(&DeviceClient::defaultSendCallbackImpl);
    m_msgReceiveCallback = MakeCallback(&DeviceClient::defaultReceiveCallbackImpl);
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
    if (m_socket == 0) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        } else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(m_peerAddress);
        } else {
            NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
        }
    }
    m_socket->SetRecvCallback(MakeCallback(&DeviceClient::HandleRead, this));
    m_socket->SetAllowBroadcast(true);
    ScheduleTransmit(Seconds(0.));
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
DeviceClient::ScheduleTransmit(Time dt) {
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &DeviceClient::Send, this);
}

void
DeviceClient::Send(void) {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    // Packet with message and timestamp.
    string msg = m_msgSendCallback(m_fromNodeId, m_toNodeId);
    Ptr<Packet> p = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);

    // Creates one with the current timestamp
    SeqTsHeader seqTs;
    seqTs.SetSeq(m_sent);
    p->AddHeader(seqTs);

    // Timestamps
    m_sendRequestTimestamps.push_back(Simulator::Now().GetNanoSeconds());
    m_replyTimestamps.push_back(-1);
    m_receiveReplyTimestamps.push_back(-1);
    m_sent++;

    // Send out
    m_socket->Send(p);

    // Schedule next transmit
    ScheduleTransmit(m_interval);
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
        uint32_t seqNo = incomingSeqTs.GetSeq();

        // Update the local timestamps
        m_replyTimestamps[seqNo] = incomingSeqTs.GetTs().GetNanoSeconds();
        m_receiveReplyTimestamps[seqNo] = Simulator::Now().GetNanoSeconds();

        uint8_t *buffer = new uint8_t[packet->GetSize ()];
        packet->CopyData(buffer, packet->GetSize ());
        string s = string(buffer, buffer+packet->GetSize());
        m_msgReceiveCallback(s, m_fromNodeId, m_toNodeId);
        NS_LOG_DEBUG ("Buffer: size = " << packet->GetSize() << " - content = >>" << s << "<<");
        delete buffer;
    }
}

uint64_t DeviceClient::GetFromNodeId() {
    return m_fromNodeId;
}

uint64_t DeviceClient::GetToNodeId() {
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
