#ifndef PROCESSING_TIME_H
#define PROCESSING_TIME_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{
    
class ProcessingTime : public Object {
    
public:
    
    ProcessingTime(Time mean, Time stdDev);
    ~ProcessingTime() {}
    
    Time GetValue() const;

    static void setStreamBaseId(int64_t sbid) { m_nextStreamId = sbid; }
    
private:
    
    static int64_t getNextStreamId() { return m_nextStreamId++; }
    static int64_t m_nextStreamId;

    Ptr<GammaRandomVariable> m_randDist;
};
    
} // namespace ns3

#endif // PROCESSING_TIME_H
