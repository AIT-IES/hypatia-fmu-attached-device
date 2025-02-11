#include "fmu-attached-device-factory.h"

#include "ns3/exp-util.h"
#include "ns3/fmu-attached-device-helper.h"

#include <common/FMIPPConfig.h>
#include <import/base/include/ModelManager.h>
#include <import/base/include/FMUModelExchange_v2.h>

using namespace std;
using namespace fmi_2_0;

namespace ns3 {

FmuAttachedDeviceFactory::FmuAttachedDeviceFactory(
    Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology
) {
    initFmuDeviceFactory(basicSimulation, topology, 
        MakeCallback(&FmuAttachedDevice::defaultInitCallbackImpl), 
        MakeCallback(&FmuAttachedDevice::defaultDoStepCallbackImpl));
}

FmuAttachedDeviceFactory::FmuAttachedDeviceFactory(
    Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
    FmuAttachedDevice::DoStepCallbackType doStepCallback
) {
    initFmuDeviceFactory(basicSimulation, topology, 
        MakeCallback(&FmuAttachedDevice::defaultInitCallbackImpl), doStepCallback);
}

FmuAttachedDeviceFactory::FmuAttachedDeviceFactory(
    Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
    FmuAttachedDevice::InitCallbackType initCallback, 
    FmuAttachedDevice::DoStepCallbackType doStepCallback
) {
    initFmuDeviceFactory(basicSimulation, topology, initCallback, doStepCallback);
}

void
FmuAttachedDeviceFactory::initFmuDeviceFactory(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
        FmuAttachedDevice::InitCallbackType initCallback, FmuAttachedDevice::DoStepCallbackType doStepCallback)
{
    printf("FMU DEVICE FACTORY\n");

    m_basicSimulation = basicSimulation;
    m_topology = topology;

    // Check if it is enabled explicitly
    m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_fmu_attached_devices", "false"));
    if (!m_enabled) {
        std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

    } else {
        std::cout << "  > FMU device factory is enabled" << std::endl;

        m_nodes = m_topology->GetNodes();

        string fmuConfigRaw = basicSimulation->GetConfigParamOrFail("fmu_config_files");
        vector<pair<string, string>> fmuConfigList = parse_map_string(fmuConfigRaw);
        vector<pair<string, string>>::const_iterator itFmuConfigList;
        for (itFmuConfigList = fmuConfigList.begin(); itFmuConfigList != fmuConfigList.end(); ++itFmuConfigList)
        {
            int64_t endpoint = parse_positive_int64(itFmuConfigList->first);
            string fmuConfigFileName = basicSimulation->GetRunDir() + "/" + itFmuConfigList->second;

            printf("  > Read FMU configuration for enpoint %ld from %s\n", endpoint, fmuConfigFileName.c_str());
            map<string, string> fmuConfig = read_config(fmuConfigFileName);

            string fmuDirUri = get_param_or_fail("fmu_dir_uri", fmuConfig);
            printf("    >> Load extracted FMU from %s\n", fmuDirUri.c_str());

            NS_ABORT_MSG_UNLESS(dir_exists(getPathFromFileUri(fmuDirUri)), 
                format_string("Not a directory: %s", getPathFromFileUri(fmuDirUri).c_str()));
            NS_ABORT_MSG_UNLESS(file_exists(getPathFromFileUri(fmuDirUri) + "/modelDescription.xml"),
                "Not a valid FMU: no model descritpion found");
            NS_ABORT_MSG_UNLESS(dir_exists(getPathFromFileUri(fmuDirUri) + "/binaries"), 
                "Not a valid FMU: no binaries folder found");

            double fmuStartTimeInS = parse_positive_double(get_param_or_default("start_time_in_s", "0.0", fmuConfig));
            double fmuCommStepSizeInS = parse_positive_double(get_param_or_fail("comm_step_size_in_s", fmuConfig));
            bool loggingOn = toBoolean(get_param_or_fail("logging_on", fmuConfig));
            string modelIdentifier = get_param_or_fail("model_identifier", fmuConfig);

            ModelManager::LoadFMUStatus status = ModelManager::failed;
            FMUType type = invalid;
            status = ModelManager::loadFMU(fmuDirUri, loggingOn, type, modelIdentifier);
            
            NS_ABORT_MSG_UNLESS(status == ModelManager::success, "Loading of FMU failed");
            NS_ABORT_MSG_UNLESS(type == fmi_2_0_cs, "Wrong FMU type");

            printf("    >> FMU loaded successfully\n");

            // Helper to install the application.
            FmuAttachedDeviceHelper fmuDevice(1025, endpoint, modelIdentifier, fmuStartTimeInS, fmuCommStepSizeInS, loggingOn, initCallback, doStepCallback);

            printf("    >> FMU instance successfully attached to device\n");

            printf("  > Read FMU configuration for writing results\n");
            bool fmuResultsWrite = toBoolean(get_param_or_fail("fmu_res_write", fmuConfig));
            if (fmuResultsWrite) {
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

            // Install it on the node and start it right now
            ApplicationContainer app = fmuDevice.Install(m_nodes.Get(endpoint));
            app.Start(Seconds(0.0));
            m_apps.push_back(app);
        }
        m_basicSimulation->RegisterTimestamp("Setup FMU-attached devices");

    }

    std::cout << std::endl;
}

string FmuAttachedDeviceFactory::getPathFromFileUri(const string& uri) const
{
    if ( uri.substr(0, 7) != "file://" ) {
        throw runtime_error(format_string("Invalid file URI: %s", uri.c_str()));
    }

	return uri.substr(7, uri.size());
}

bool FmuAttachedDeviceFactory::toBoolean(const std::string & v) const
{
    return !v.empty () &&
        (strcasecmp (v.c_str (), "true") == 0 ||
         atoi (v.c_str ()) != 0);
}


}
