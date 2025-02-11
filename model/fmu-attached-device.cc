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
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/exp-util.h"

#include "fmu-attached-device.h"

#include <cmath>
#include <fstream>

using namespace fmi_2_0;
using namespace std;

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("FmuAttachedDevice");

    NS_OBJECT_ENSURE_REGISTERED (FmuAttachedDevice);

    TypeId
    FmuAttachedDevice::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::FmuAttachedDevice")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<FmuAttachedDevice>()
                .AddAttribute("Port", "Port on which we listen for incoming packets.",
                              UintegerValue(9),
                              MakeUintegerAccessor(&FmuAttachedDevice::m_port),
                              MakeUintegerChecker<uint16_t>())
                .AddAttribute("NodeId", "Node identifier",
                              UintegerValue(0),
                              MakeUintegerAccessor(&FmuAttachedDevice::m_nodeId),
                              MakeUintegerChecker<uint64_t>())
                .AddAttribute("ModelIdentifier", "FMU model identifier.",
                              StringValue(),
                              MakeStringAccessor(&FmuAttachedDevice::m_modelIdentifier),
                              MakeStringChecker())
                .AddAttribute("ModelStepSize", "Set the communication step size for the FMU (in seconds).",
                              DoubleValue(1e-5),
                              MakeDoubleAccessor(&FmuAttachedDevice::m_commStepSizeInS),
                              MakeDoubleChecker<double>(numeric_limits<double>::min()))
                .AddAttribute("ModelStartTime", "Set the start time for the FMU (in seconds).",
                              DoubleValue(0.),
                              MakeDoubleAccessor(&FmuAttachedDevice::m_startTimeInS),
                              MakeDoubleChecker<double>(0.))
                .AddAttribute("LoggingOn", "Turn on logging for FMU.",
                              BooleanValue(false),
                              MakeBooleanAccessor(&FmuAttachedDevice::m_loggingOn),
                              MakeBooleanChecker())
                .AddAttribute("InitCallback",
                              "Callback for instantiating and initializing the FMU model.",
                              CallbackValue(MakeCallback(&FmuAttachedDevice::defaultInitCallbackImpl)),
                              MakeCallbackAccessor(&FmuAttachedDevice::m_initCallback),
                              MakeCallbackChecker())
                .AddAttribute("DoStepCallback",
                              "Callback for performing a simulation step and returning a payload message.",
                              CallbackValue(MakeCallback(&FmuAttachedDevice::defaultDoStepCallbackImpl)),
                              MakeCallbackAccessor(&FmuAttachedDevice::m_doStepCallback),
                              MakeCallbackChecker())
                .AddAttribute("ResultsWrite",
                              "Flag to indicate if results file should be written.",
                              BooleanValue(false),
                              MakeBooleanAccessor(&FmuAttachedDevice::m_resWrite),
                              MakeBooleanChecker())
                .AddAttribute("ResultsWritePeriodInS",
                              "Time period to write values to results file.",
                              DoubleValue(1.),
                              MakeDoubleAccessor(&FmuAttachedDevice::m_resWritePeriodInS),
                              MakeDoubleChecker<double>(numeric_limits<double>::min()))
                .AddAttribute("ResultsFilename",
                              "Name of results file.",
                              StringValue(),
                              MakeStringAccessor (&FmuAttachedDevice::m_resFilename),
                              MakeStringChecker())
                .AddAttribute("ResultsVariableNamesList",
                              "List of names of variables whose values should be written to the results file.",
                              StringValue(),
                              MakeStringAccessor (&FmuAttachedDevice::m_resVarnamesList),
                              MakeStringChecker());
        return tid;
    }

    FmuAttachedDevice::FmuAttachedDevice() {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
        m_fmu = 0;
    }

    FmuAttachedDevice::~FmuAttachedDevice() {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
        m_fmu = 0;
    }


    void
    FmuAttachedDevice::defaultInitCallbackImpl(
        Ptr<RefFMU> fmu, uint64_t nodeId, 
        const std::string& modelIdentifier, const double& startTime
    ) {
        fmippStatus status = fmippFatal;

        // Instantiate the FMU model.
        status = fmu->instantiate(modelIdentifier + to_string(nodeId), 0., false, false);
        NS_ABORT_MSG_UNLESS(status == fmippOK, "instantiation of FMU failed");

        // Initialize the FMU model.
        status = fmu->initialize(startTime, false, numeric_limits<double>::max());
        NS_ABORT_MSG_UNLESS(status == fmippOK, "initialization of FMU failed");
    }

    string
    FmuAttachedDevice::defaultDoStepCallbackImpl(
        Ptr<RefFMU> fmu, uint64_t nodeId, const std::string& payload, const double& time, const double& commStepSize
    ) {
        // Integrate FMU model until current simulation time is reached.
        // Iterate in case events are encountered.
        double tt = fmu->getTime();
        do {
            fmippStatus status = fmu->doStep(tt, commStepSize, true);
            NS_ABORT_MSG_UNLESS(status == fmippOK, "stepping of FMU failed");
            tt += commStepSize;
        } while (tt < time);

        // Return default message.
        return string("FMU model stepped until t=") + to_string(tt);
    }
    
    void
    FmuAttachedDevice::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void
    FmuAttachedDevice::StartApplication(void) {
        NS_LOG_FUNCTION(this);

        if (m_socket == 0) {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            if (m_socket->Bind(local) == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            if (addressUtils::IsMulticast(m_local)) {
                throw std::runtime_error("Multi-cast is not supported.");
            }
        }

        if (m_fmu == 0 && !m_modelIdentifier.empty()) {
            // Load FMU.
            m_fmu = CreateObject<RefFMU>(m_modelIdentifier, m_loggingOn);

            // Instantiate and initialize FMU via callback.
            m_initCallback(m_fmu, m_nodeId, m_modelIdentifier, m_startTimeInS);
        }

        m_socket->SetRecvCallback(MakeCallback(&FmuAttachedDevice::HandleRead, this));

        // Initialize periodic writing of FMU model data.
        if (m_resWrite) {
            // Clean-up previously written results.
            remove_file_if_exists(m_resFilename);

            // Parse list to vector.
            m_resVarnames = parse_list_string(m_resVarnamesList);

            // Schedule the next write event.
            m_writeDataEvent = Simulator::Schedule(Simulator::Now(), &FmuAttachedDevice::WriteData, this);
        }
    }

    void
    FmuAttachedDevice::StopApplication() {
        NS_LOG_FUNCTION(this);
        if (m_socket != 0) {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        }
        Simulator::Cancel(m_writeDataEvent);
    }

    void
    FmuAttachedDevice::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packetIn;
        Ptr<Packet> packetOut;
        Address from;
        while ((packetIn = socket->RecvFrom(from))) {

            NS_LOG_DEBUG ("At time " << Simulator::Now ().GetSeconds () << "s device received " << packetIn->GetSize () << " bytes from " <<
                InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                InetSocketAddress::ConvertFrom (from).GetPort ());

            // What we receive
            SeqTsHeader incomingSeqTs;
            packetIn->RemoveHeader(incomingSeqTs);

            uint8_t *buffer = new uint8_t[packetIn->GetSize ()];
            packetIn->CopyData(buffer, packetIn->GetSize ());
            string payload = string(buffer, buffer+packetIn->GetSize());
            NS_LOG_DEBUG ("Buffer: size = " << packetIn->GetSize() << " - content: " << payload);
            delete buffer;

            double t = Simulator::Now().GetSeconds();
            string msg = m_doStepCallback(m_fmu, m_nodeId, payload, t, m_commStepSizeInS);
            packetOut = Create<Packet>((uint8_t*) msg.c_str(), msg.length() + 1);

            // Add header
            SeqTsHeader outgoingSeqTs; // Creates one with the current timestamp
            outgoingSeqTs.SetSeq(incomingSeqTs.GetSeq());
            packetOut->AddHeader(outgoingSeqTs);

            // Send back with the new timestamp on it
            socket->SendTo(packetOut, 0, from);
        }
    }

    void
    FmuAttachedDevice::WriteData() {
        NS_ASSERT(m_writeDataEvent.IsExpired());

        // Sync FMU model with current time step.
        double t = Simulator::Now().GetSeconds();
        string msg = defaultDoStepCallbackImpl(m_fmu, m_nodeId, "", t, m_commStepSizeInS);

        // Open the file in append mode.
        ofstream file(m_resFilename, ios::app);

        if (!file.is_open()) {
            NS_FATAL_ERROR ("Failed to open file: " << m_resFilename);
        }

        // If the file is empty, write the column names.
        file.seekp(0, ios::end);
        if (0 == file.tellp()) {
            // Name of time column.
            file << "time";
            if (m_resVarnames.size()) { file << ","; }

            // Names of all other columns.
            for (size_t i = 0; i < m_resVarnames.size(); ++i) {
                file << m_resVarnames[i];
                if (i < m_resVarnames.size() - 1) { file << ","; }
            }
            file << "\n";
        }

        // Write current timestamp (simulation in seconds).
        file << t;
        if (m_resVarnames.size()) { file << ","; }

        // Write all other rows.
        for (size_t i = 0; i < m_resVarnames.size(); ++i)
        {
            switch(m_fmu->getType(m_resVarnames[i])) {
            case fmippTypeReal:
                file << m_fmu->getRealValue(m_resVarnames[i]);
                break;
            case fmippTypeInteger:
                file << m_fmu->getIntegerValue(m_resVarnames[i]);
                break;
            case fmippTypeBoolean:
                file << m_fmu->getBooleanValue(m_resVarnames[i]);
                break;
            case fmippTypeString:
                file << m_fmu->getStringValue(m_resVarnames[i]);
                break;
            case fmippTypeUnknown:
                break;
            }
            
            if (i < m_resVarnames.size() - 1) { file << ","; }
        }
        file << "\n";

        // Close the file.
        file.close();

        if (!file) {
            NS_FATAL_ERROR ("Error occurred while writing to file: " << m_resFilename);
        } else {
            NS_LOG_DEBUG ("Data successfully written to " << m_resFilename);
        }

        // Schedule the next write event.
        m_writeDataEvent = Simulator::Schedule(Time(Seconds(m_resWritePeriodInS)), &FmuAttachedDevice::WriteData, this);
    }

} // Namespace ns3
