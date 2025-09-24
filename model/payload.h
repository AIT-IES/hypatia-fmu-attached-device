#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <string>

namespace ns3 {

class Payload
{
public:

    Payload();
    Payload(const std::string& buffer);
    Payload(uint32_t buffer_size);

    bool IsValid() const { return (m_id != INVALID); }
    uint32_t GetId() const { return m_id; }
    const std::string& GetBuffer() const { return m_buffer; }
    uint32_t GetBufferSize() const { return m_buffer_size; }
    bool GetTransmitBuffer() const { return m_transmit_buffer; }

    enum PayloadId {
        INVALID = 0,
        FIRST = 1
    };

private:    
    
    uint32_t m_id;
    std::string m_buffer;
    uint32_t m_buffer_size;
    bool m_transmit_buffer;

    static uint32_t m_global_payload_id;
};

}

#endif // PAYLOAD_H