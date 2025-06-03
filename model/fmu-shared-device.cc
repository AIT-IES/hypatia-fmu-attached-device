#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/integer.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/exp-util.h"
#include "ns3/fmu-util.h"

#include "fmu-shared-device.h"

#include <cmath>
#include <fstream>

using namespace fmi_2_0;
using namespace std;

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("FmuSharedDevice");

    NS_OBJECT_ENSURE_REGISTERED (FmuSharedDevice);

    // Initialize empty collection of shared FMUs.
    FmuSharedDevice::SharedFmuCollection FmuSharedDevice::m_sharedFmuCollection = FmuSharedDevice::SharedFmuCollection();

    TypeId
    FmuSharedDevice::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::FmuSharedDevice")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<FmuSharedDevice>()
            .AddAttribute("Port", "Port on which we listen for incoming packets.",
                          UintegerValue(9),
                          MakeUintegerAccessor(&FmuSharedDevice::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("NodeId", "Node identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&FmuSharedDevice::m_nodeId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("ModelIdentifier", "FMU model identifier.",
                          StringValue(),
                          MakeStringAccessor(&FmuSharedDevice::m_modelIdentifier),
                          MakeStringChecker())
            .AddAttribute("ModelStepSize", "Set the communication step size for the FMU (in seconds).",
                          DoubleValue(1e-5),
                          MakeDoubleAccessor(&FmuSharedDevice::m_commStepSizeInS),
                          MakeDoubleChecker<double>(numeric_limits<double>::min()))
            .AddAttribute("ModelStartTime", "Set the start time for the FMU (in seconds).",
                          DoubleValue(0.),
                          MakeDoubleAccessor(&FmuSharedDevice::m_startTimeInS),
                          MakeDoubleChecker<double>(0.))
            .AddAttribute("LoggingOn", "Turn on logging for FMU.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&FmuSharedDevice::m_loggingOn),
                          MakeBooleanChecker())
            .AddAttribute("InitCallback",
                          "Callback for instantiating and initializing the FMU model.",
                          CallbackValue(MakeCallback(&FmuSharedDevice::defaultInitCallbackImpl)),
                          MakeCallbackAccessor(&FmuSharedDevice::m_initCallback),
                          MakeCallbackChecker())
            .AddAttribute("DoStepCallback",
                          "Callback for performing a simulation step and returning a payload message.",
                          CallbackValue(MakeCallback(&FmuSharedDevice::defaultDoStepCallbackImpl)),
                          MakeCallbackAccessor(&FmuSharedDevice::m_doStepCallback),
                          MakeCallbackChecker())
            .AddAttribute("ResultsWrite",
                          "Flag to indicate if results file should be written.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&FmuSharedDevice::m_resWrite),
                          MakeBooleanChecker())
            .AddAttribute("ResultsWritePeriodInS",
                          "Time period to write values to results file.",
                          DoubleValue(1.),
                          MakeDoubleAccessor(&FmuSharedDevice::m_resWritePeriodInS),
                          MakeDoubleChecker<double>(numeric_limits<double>::min()))
            .AddAttribute("ResultsFilename",
                          "Name of results file.",
                          StringValue(),
                          MakeStringAccessor (&FmuSharedDevice::m_resFilename),
                          MakeStringChecker())
            .AddAttribute("ResultsVariableNamesList",
                          "List of names of variables whose values should be written to the results file.",
                          StringValue(),
                          MakeStringAccessor (&FmuSharedDevice::m_resVarnamesList),
                          MakeStringChecker())
            .AddAttribute("SharedFmuInstanceName",
                          "Common name of the shared FMU instance.",
                          StringValue(),
                          MakeStringAccessor (&FmuSharedDevice::m_sharedFmuInstanceName),
                          MakeStringChecker())
            .AddAttribute("ProcessingTimeMean",
                          "Average processing time",
                          TimeValue(MilliSeconds(1)),
                          MakeTimeAccessor(&FmuSharedDevice::m_processingTimeMean),
                          MakeTimeChecker())
            .AddAttribute("ProcessingTimeStdDev",
                          "Standard deviation of processing time",
                          TimeValue(MicroSeconds(50)),
                          MakeTimeAccessor(&FmuSharedDevice::m_processingTimeStdDev),
                          MakeTimeChecker());
        return tid;
    }

    void
    FmuSharedDevice::initFmu() {
        NS_ABORT_MSG_UNLESS(false == m_sharedFmuInstanceName.empty(), "No common name for shared FMU instance provided.");

        SharedFmuCollection::const_iterator itFind = m_sharedFmuCollection.find(m_sharedFmuInstanceName);
        bool fmuLoaded = (itFind != m_sharedFmuCollection.end());

        if (true == fmuLoaded) {
            m_fmu = itFind->second;
        } else {
            // Load FMU.
            m_fmu = CreateObject<RefFMU>(m_modelIdentifier, m_sharedFmuInstanceName, m_loggingOn);

            // Instantiate and initialize FMU via callback.
            m_initCallback(m_fmu, m_nodeId, m_modelIdentifier, m_startTimeInS);

            m_sharedFmuCollection[m_sharedFmuInstanceName] = m_fmu;
        }
    }

    string
    FmuSharedDevice::stepFmu(const std::string& payload, const double& t) {
        // Lock the FMU's mutex to avoid race conditions.
        // This should not be necessary, but better safe than sorry ...
        m_fmu->lock();
        
        string msg = m_doStepCallback(m_fmu, m_nodeId, payload, t, m_commStepSizeInS);
        
        // Unlock the FMU's mutex.
        m_fmu->unlock();

        return msg;
    }

} // Namespace ns3
