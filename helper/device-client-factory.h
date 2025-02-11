#ifndef DEVICE_CLIENT_FACTORY_H
#define DEVICE_CLIENT_FACTORY_H

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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/command-line.h"
#include "ns3/basic-simulation.h"
#include "ns3/exp-util.h"
#include "ns3/topology.h"
#include "ns3/device-client.h"

using namespace ns3;

namespace ns3 {

class DeviceClientFactory
{

public:
    DeviceClientFactory(
        Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
        DeviceClient::MessageSendCallbackType send_callback,
        DeviceClient::MessageReceiveCallbackType receive_callback);
    void WriteResults();

protected:
    Ptr<BasicSimulation> m_basicSimulation;
    int64_t m_simulation_end_time_ns;
    Ptr<Topology> m_topology = nullptr;
    bool m_enabled;

    NodeContainer m_nodes;
    std::vector<ApplicationContainer> m_apps;
    int64_t m_interval_ns;
    std::vector<std::pair<int64_t, int64_t>> m_endpoint_pairs;
    uint32_t m_system_id;
    bool m_enable_distributed;
    std::vector<int64_t> m_distributed_node_system_id_assignment;
    std::string m_device_csv_filename;
    std::string m_device_txt_filename;
};

}

#endif /* DEVICE_CLIENT_FACTORY_H */
