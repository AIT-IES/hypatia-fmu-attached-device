#ifndef SEND_REPLY_CONTEXT_H 
#define SEND_REPLY_CONTEXT_H

#include "ns3/address.h"
#include "ns3/object.h"

namespace ns3 {

    class Socket;
    class Packet;
    
    struct SendReplyContext : public Object {
        const Ptr<Socket> m_socket;
        const Ptr<Packet> m_packet;
        const Address m_address;
        
        SendReplyContext(const Ptr<Socket>& s, const Ptr<Packet>& p, const Address& a) :
        m_socket(s), m_packet(p), m_address(a) {}
    };
    
}

#endif // SEND_REPLY_CONTEXT_H
