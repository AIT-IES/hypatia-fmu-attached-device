#ifndef FMU_UTIL_H
#define FMU_UTIL_H

#include "ns3/object.h"

#include <import/base/include/FMUCoSimulation_v2.h>

#include <mutex>

namespace ns3 {

class RefFMU: public fmi_2_0::FMUCoSimulation, public Object
{
public:
    RefFMU(const std::string& modelIdentifier, const std::string& instanceName, const bool loggingOn):
        FMUCoSimulation(modelIdentifier, loggingOn), Object(), m_instanceName(instanceName) {}
    
    inline const std::string& instanceName() const { return m_instanceName; }

    // Provide a mutex to avoid race conditions.
    // This should not be necessary, but better safe than sorry ...
    inline void lock() { m_mtx.lock(); }
    inline void unlock() { m_mtx.unlock(); }

private:
    const std::string m_instanceName;
    std::mutex m_mtx;
};

}

#endif // FMU_UTIL_H