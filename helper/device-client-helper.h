#ifndef DEVICE_CLIENT_HELPER_H
#define DEVICE_CLIENT_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/device-client.h"

namespace ns3 {

class DeviceClientHelper
{
public:
  typedef DeviceClient::MessageSendCallbackType MessageSendCallbackType;
  typedef DeviceClient::MessageReceiveCallbackType MessageReceiveCallbackType;

  DeviceClientHelper (
    Address ip, uint16_t port, uint64_t from_node_id, uint64_t to_node_id, 
    MessageSendCallbackType send_callback, MessageReceiveCallbackType receive_callback,
    Time processing_time_const = Seconds(0), 
    Time processing_time_mean = MilliSeconds(1), 
    Time processing_time_std_dev = MicroSeconds(50));
  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (Ptr<Node> node) const;

private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* DEVICE_CLIENT_HELPER_H */
