#ifndef FMU_ATTACHED_DEVICE_HELPER_H
#define FMU_ATTACHED_DEVICE_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/fmu-attached-device.h"

namespace ns3 {

class FmuAttachedDeviceHelper
{
public:
  typedef FmuAttachedDevice::InitCallbackType InitCallbackType;
  typedef FmuAttachedDevice::DoStepCallbackType DoStepCallbackType;

  FmuAttachedDeviceHelper (uint16_t port, uint16_t nodeId, const std::string& modelIdentifier,
    const double& startTime, const double& commStepSize, bool loggingOn);

  FmuAttachedDeviceHelper (uint16_t port, uint16_t nodeId, const std::string& modelIdentifier,
    const double& startTime, const double& commStepSize, bool loggingOn, 
    DoStepCallbackType doStepCallback);

  FmuAttachedDeviceHelper (uint16_t port, uint16_t nodeId, const std::string& modelIdentifier,
    const double& startTime, const double& commStepSize, bool loggingOn, 
    InitCallbackType initCallback, DoStepCallbackType doStepCallback);

  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (Ptr<Node> node) const;
  ApplicationContainer Install (NodeContainer c) const;

private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* FMU_ATTACHED_DEVICE_HELPER_H */
