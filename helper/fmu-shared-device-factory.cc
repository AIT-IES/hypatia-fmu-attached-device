#include "fmu-shared-device-factory.h"
#include "fmu-device-helper.h"
#include "factory-util.h"

#include "ns3/exp-util.h"

#include <common/FMIPPConfig.h>
#include <import/base/include/ModelManager.h>
#include <import/base/include/FMUModelExchange_v2.h>

#include <boost/filesystem.hpp>
#include <sstream>

using namespace std;
using namespace fmi_2_0;
using namespace boost::filesystem;

namespace {

string getFileUriFromPath(const path& path)
{
    stringstream str;
    str << "file://" << path.string();
	return str.str();
}

bool toBoolean(const std::string & v)
{
    return !v.empty () &&
        (strcasecmp (v.c_str (), "true") == 0 ||
         atoi (v.c_str ()) != 0);
}

void checkEndpointPairs(
    const std::vector<std::pair<int64_t, int64_t>>& endpoint_pairs,
    const std::set<int64_t>& shared_endpoints
) {
    for (auto p : endpoint_pairs)
    {
        std::set<int64_t>::const_iterator find = shared_endpoints.find(p.first);
        NS_ABORT_MSG_UNLESS(
            find != shared_endpoints.end(),
            format_string("Not a shared endpoint: %ld", p.first)
        );
    }
}

}

namespace ns3 {

FmuSharedDeviceFactory::FmuSharedDeviceFactory(
    Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology
) {
    initFmuDeviceFactory(basicSimulation, topology, 
        MakeCallback(&FmuSharedDevice::defaultInitCallbackImpl), 
        MakeCallback(&FmuSharedDevice::defaultDoStepCallbackImpl));
}

FmuSharedDeviceFactory::FmuSharedDeviceFactory(
    Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
    FmuSharedDevice::DoStepCallbackType doStepCallback
) {
    initFmuDeviceFactory(basicSimulation, topology, 
        MakeCallback(&FmuSharedDevice::defaultInitCallbackImpl), doStepCallback);
}

FmuSharedDeviceFactory::FmuSharedDeviceFactory(
    Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
    FmuSharedDevice::InitCallbackType initCallback, 
    FmuSharedDevice::DoStepCallbackType doStepCallback
) {
    initFmuDeviceFactory(basicSimulation, topology, initCallback, doStepCallback);
}

void
FmuSharedDeviceFactory::initFmuDeviceFactory(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
        FmuSharedDevice::InitCallbackType initCallback, FmuSharedDevice::DoStepCallbackType doStepCallback)
{
    printf("FMU SHARED DEVICE FACTORY\n");

    m_basicSimulation = basicSimulation;
    m_topology = topology;

    // Check if it is enabled explicitly
    m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_fmu_shared_devices", "false"));
    if (!m_enabled) {
        std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

    } else {
        std::cout << "  > FMU shared device factory is enabled" << std::endl;

        m_nodes = m_topology->GetNodes();

        string fmuConfigRaw = basicSimulation->GetConfigParamOrFail("fmu_config_files");
        vector<pair<string, string>> fmuConfigList = parse_map_string(fmuConfigRaw);
        for (auto const& config: fmuConfigList)
        {
            string fmuConfigFileName = basicSimulation->GetRunDir() + "/" + config.second;

            printf("  > Read FMU configuration from %s\n", fmuConfigFileName.c_str());
            map<string, string> fmuConfig = read_config(fmuConfigFileName);

            path fmuDir = get_param_or_fail("fmu_dir", fmuConfig);
            path fmuDirAbs = fmuDir.is_absolute() ? fmuDir :
                canonical(absolute(fmuDir, basicSimulation->GetRunDir()));

            printf("    >> Load extracted FMU from %s\n", fmuDirAbs.string().c_str());

            NS_ABORT_MSG_UNLESS(dir_exists(fmuDirAbs.string().c_str()), 
                format_string("Not a directory: %s", fmuDirAbs.string().c_str()));
            NS_ABORT_MSG_UNLESS(file_exists((fmuDirAbs / "/modelDescription.xml").string().c_str()),
                "Not a valid FMU: no model descritpion found");
            NS_ABORT_MSG_UNLESS(dir_exists((fmuDirAbs / "/binaries").string().c_str()), 
                "Not a valid FMU: no binaries folder found");

            double fmuStartTimeInS = parse_positive_double(get_param_or_default("start_time_in_s", "0.0", fmuConfig));
            double fmuCommStepSizeInS = parse_positive_double(get_param_or_fail("comm_step_size_in_s", fmuConfig));
            bool loggingOn = toBoolean(get_param_or_fail("logging_on", fmuConfig));
            string modelIdentifier = get_param_or_fail("model_identifier", fmuConfig);
            string sharedFmuInstanceName = get_param_or_fail("shared_instance_name", fmuConfig);
            double proc_time_const_ns = parse_positive_double(get_param_or_fail("processing_time_const_ns", fmuConfig));
            double proc_time_mean_ns = parse_positive_double(get_param_or_fail("processing_time_mean_ns", fmuConfig));
            double proc_time_std_dev_ns = parse_positive_double(get_param_or_fail("processing_time_std_dev_ns", fmuConfig));

            string fmuDirUri = getFileUriFromPath(fmuDirAbs);
            ModelManager::LoadFMUStatus status = ModelManager::failed;
            FMUType type = invalid;
            status = ModelManager::loadFMU(fmuDirUri, loggingOn, type, modelIdentifier);
            
            NS_ABORT_MSG_UNLESS(status == ModelManager::success || status == ModelManager::duplicate, "Loading of FMU failed");
            NS_ABORT_MSG_UNLESS(type == fmi_2_0_cs, "Wrong FMU type");

            printf("    >> FMU loaded successfully\n");

            std::string config_shared_endpoints = basicSimulation->GetConfigParamOrFail(config.first);
            std::set<int64_t> shared_endpoints = parse_set_positive_int64(config_shared_endpoints);

            bool sendData = parse_boolean(get_param_or_default("send_data", "false", fmuConfig));
            double sendDataInterval = parse_positive_double(get_param_or_default("send_data_interval_s", "1.0", fmuConfig));

            // Parse pairs of connected endpoints.
            std::vector<std::pair<int64_t, int64_t>> send_data_endpoints =
                parse_endpoint_pairs(get_param_or_default("send_data_endpoints", "set()", fmuConfig), topology);
            checkEndpointPairs(send_data_endpoints, shared_endpoints);

            bool firstEndpoint = true;
            for (int64_t endpoint : shared_endpoints)
            {
                printf("  > Attach FMU to enpoint %ld\n", endpoint);

                // Helper to install the application.
                FmuDeviceHelper<FmuSharedDevice> fmuDevice(1025, endpoint, modelIdentifier, fmuStartTimeInS,
                    fmuCommStepSizeInS, loggingOn, initCallback, doStepCallback,
                    NanoSeconds(proc_time_const_ns), NanoSeconds(proc_time_mean_ns), NanoSeconds(proc_time_std_dev_ns));
                fmuDevice.SetAttribute("SharedFmuInstanceName", StringValue(sharedFmuInstanceName));

                printf("    >> Shared FMU instance successfully attached to device\n");

                bool fmuResultsWrite = toBoolean(get_param_or_fail("fmu_res_write", fmuConfig));
                if (fmuResultsWrite && firstEndpoint) // Only write results for first endpoint (avoid duplicate results).
                {
                    printf("  > Read FMU configuration for writing results\n");
                    fmuDevice.SetAttribute("ResultsWrite", BooleanValue(fmuResultsWrite));

                    double fmuResWritePeriodInS = parse_positive_double(get_param_or_fail("fmu_res_write_period_in_s", fmuConfig));
                    fmuDevice.SetAttribute("ResultsWritePeriodInS", DoubleValue(fmuResWritePeriodInS));
                    printf("    >> writing results every %f seconds\n", fmuResWritePeriodInS);

                    string fmuResultsFilename = get_param_or_fail("fmu_res_filename", fmuConfig);
                    fmuResultsFilename = basicSimulation->GetLogsDir() + "/" + fmuResultsFilename;
                    fmuDevice.SetAttribute("ResultsFilename", StringValue(fmuResultsFilename));
                    printf("    >> writing results to: %s\n", fmuResultsFilename.c_str());

                    string fmuResultsVarnamesList = get_param_or_fail("fmu_res_varnames", fmuConfig);
                    fmuDevice.SetAttribute("ResultsVariableNamesList", StringValue(fmuResultsVarnamesList));
                    printf("    >> writing values of the following variables: %s\n", fmuResultsVarnamesList.c_str());
                } else {
                    printf("    >> not writing any results\n");
                }

                if (sendData)
                {
                    for (std::pair<int64_t, int64_t>& p : send_data_endpoints)
                    {
                        if (endpoint != p.first) { continue; }

                        int64_t sendDataEndpoint = p.second;
                        uint32_t sendDataPort = 1025;
    
                        fmuDevice.SetAttribute("SendData", BooleanValue (sendData));
                        fmuDevice.SetAttribute("SendInterval", TimeValue(Seconds(sendDataInterval)));
                        fmuDevice.SetAttribute("RemoteAddress", AddressValue (m_nodes.Get(sendDataEndpoint)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()));
                        fmuDevice.SetAttribute("RemotePort", UintegerValue (sendDataPort));
                        
                        printf("    >> sending data every %f seconds to endpoint %ld (port %d)\n", sendDataInterval, sendDataEndpoint, sendDataPort);
                    }
                }
                
                // Install it on the node and start it right now
                ApplicationContainer app = fmuDevice.Install(m_nodes.Get(endpoint));
                app.Start(Seconds(0.0));
                m_apps.push_back(app);

                firstEndpoint = false;
            }
        }
        m_basicSimulation->RegisterTimestamp("Setup devices attached to the same FMU");

    }

    std::cout << std::endl;
}

}
