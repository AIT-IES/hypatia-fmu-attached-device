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
    DeviceClientFactory(
        Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, 
        DeviceClient::MessageReceiveCallbackType receive_callback);
    void WriteResults();

protected:
    Ptr<BasicSimulation> m_basicSimulation;
    bool m_enabled;
    bool m_send_data;
    std::vector<ApplicationContainer> m_apps;
    std::string m_device_csv_filename;
    std::string m_device_txt_filename;
};

}

#endif // DEVICE_CLIENT_FACTORY_H
