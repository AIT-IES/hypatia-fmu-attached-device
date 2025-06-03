
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/object-factory.h"
#include "processing-time.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("ProcessingTime");

// Set to 2^62 by default, for details see:
// https://www.nsnam.org/docs/manual/html/random-variables.html#setting-the-stream-number
int64_t ProcessingTime::m_nextStreamId = 4611686018427387904;

ProcessingTime::ProcessingTime(
    Time mean, Time stdDev
) {
    // Get mean and standard deviation in seconds.
    double m = mean.GetSeconds();
    double s = stdDev.GetSeconds();

    // Calculate parameters of gamma distribution.
    double alpha =  m * m / s / s;
    double beta = s * s / m;

    NS_LOG_DEBUG("init gamma distribution with alpha = " << alpha << " and beta = " << beta);

    // Create gamma distribution object.
    ObjectFactory factory;
    factory.SetTypeId(GammaRandomVariable::GetTypeId());
    factory.Set("Alpha", DoubleValue(alpha));
    factory.Set("Beta", DoubleValue(beta));
    m_randDist = factory.Create<GammaRandomVariable>();

    m_randDist->SetStream(getNextStreamId());
}

Time
ProcessingTime::GetValue() const {
    return Seconds(m_randDist->GetValue());
}

}