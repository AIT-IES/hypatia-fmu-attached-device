#include "ns3/log.h"

#include "payload.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Payload");

uint32_t Payload::m_global_payload_id = Payload::FIRST;

Payload::Payload() : 
    m_id(INVALID),
    m_buffer(), 
    m_buffer_size(0), 
    m_transmit_buffer(false)
{
    NS_LOG_FUNCTION(this << " - create payload with ID = " << m_id);
}

Payload::Payload(const std::string& buffer) : 
    m_id(m_global_payload_id),
    m_buffer(buffer), 
    m_buffer_size(buffer.length() + 1), 
    m_transmit_buffer(true)
{
    NS_LOG_FUNCTION(this << " - create payload with ID = " << m_id);
    ++m_global_payload_id;
}
    
Payload::Payload(uint32_t buffer_size) :
    m_id(m_global_payload_id),
    m_buffer(), 
    m_buffer_size(buffer_size), 
    m_transmit_buffer(false)
{
    NS_LOG_FUNCTION(this << " - create payload with ID = " << m_id);
    ++m_global_payload_id;
}

}