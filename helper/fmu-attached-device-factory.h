#ifndef FMU_ATTACHED_DEVICE_FACTORY_H
#define FMU_ATTACHED_DEVICE_FACTORY_H

#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <stdexcept>

#include "ns3/basic-simulation.h"
#include "ns3/topology.h"
#include "ns3/fmu-attached-device.h"

using namespace ns3;

namespace ns3 {

class FmuAttachedDeviceFactory
{

public:
    FmuAttachedDeviceFactory(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology);
    FmuAttachedDeviceFactory(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
        FmuAttachedDevice::DoStepCallbackType doStepCallback);
    FmuAttachedDeviceFactory(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
        FmuAttachedDevice::InitCallbackType initCallback, FmuAttachedDevice::DoStepCallbackType doStepCallback);

protected:
    Ptr<BasicSimulation> m_basicSimulation;
    Ptr<Topology> m_topology = nullptr;
    bool m_enabled;

    NodeContainer m_nodes;
    std::vector<ApplicationContainer> m_apps;

private:

    void initFmuDeviceFactory(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
        FmuAttachedDevice::InitCallbackType initCallback, FmuAttachedDevice::DoStepCallbackType doStepCallback);

	std::string getPathFromFileUri(const std::string& url) const;
    bool toBoolean(const std::string& v) const;
};

}

#endif /* FMU_ATTACHED_DEVICE_FACTORY_H */
