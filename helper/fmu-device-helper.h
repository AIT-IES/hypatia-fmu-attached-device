#ifndef FMU_DEVICE_HELPER_H
#define FMU_DEVICE_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

namespace ns3 {

template <class FmuDeviceType>
class FmuDeviceHelper
{
public:
  typedef typename FmuDeviceType::InitCallbackType InitCallbackType;
  typedef typename FmuDeviceType::DoStepCallbackType DoStepCallbackType;

  FmuDeviceHelper (uint16_t port, uint16_t nodeId, const std::string& modelIdentifier,
    const double& startTime, const double& commStepSize, bool loggingOn);

  FmuDeviceHelper (uint16_t port, uint16_t nodeId, const std::string& modelIdentifier,
    const double& startTime, const double& commStepSize, bool loggingOn, 
    DoStepCallbackType doStepCallback);

  FmuDeviceHelper (uint16_t port, uint16_t nodeId, const std::string& modelIdentifier,
    const double& startTime, const double& commStepSize, bool loggingOn, 
    InitCallbackType initCallback, DoStepCallbackType doStepCallback);

  void SetAttribute (std::string name, const AttributeValue &value);
  ApplicationContainer Install (Ptr<Node> node) const;
  ApplicationContainer Install (NodeContainer c) const;

private:
  Ptr<Application> InstallPriv (Ptr<Node> node) const;
  ObjectFactory m_factory;
};

template <class FmuDeviceType>
FmuDeviceHelper<FmuDeviceType>::FmuDeviceHelper (
  uint16_t port, uint16_t nodeId, const std::string& modelIdentifier, 
  const double& startTime, const double& commStepSize, bool loggingOn
) {
  m_factory.SetTypeId (FmuDeviceType::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("NodeId", UintegerValue (nodeId));
  SetAttribute ("ModelIdentifier", StringValue (modelIdentifier));
  SetAttribute ("ModelStartTime", DoubleValue(startTime));
  SetAttribute ("ModelStepSize", DoubleValue(commStepSize));
  SetAttribute ("LoggingOn", BooleanValue (loggingOn));
}

template <class FmuDeviceType>
FmuDeviceHelper<FmuDeviceType>::FmuDeviceHelper (
  uint16_t port, uint16_t nodeId, const std::string& modelIdentifier, 
  const double& startTime, const double& commStepSize, bool loggingOn, 
  DoStepCallbackType doStepCallback
) {
  m_factory.SetTypeId (FmuDeviceType::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("NodeId", UintegerValue (nodeId));
  SetAttribute ("ModelIdentifier", StringValue (modelIdentifier));
  SetAttribute ("ModelStartTime", DoubleValue(startTime));
  SetAttribute ("ModelStepSize", DoubleValue(commStepSize));
  SetAttribute ("LoggingOn", BooleanValue (loggingOn));
  SetAttribute ("DoStepCallback", CallbackValue (doStepCallback));
}

template <class FmuDeviceType>
FmuDeviceHelper<FmuDeviceType>::FmuDeviceHelper (
  uint16_t port, uint16_t nodeId, const std::string& modelIdentifier, 
  const double& startTime, const double& commStepSize, bool loggingOn, 
  InitCallbackType initCallback, DoStepCallbackType doStepCallback
) {
  m_factory.SetTypeId (FmuDeviceType::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("NodeId", UintegerValue (nodeId));
  SetAttribute ("ModelIdentifier", StringValue (modelIdentifier));
  SetAttribute ("ModelStartTime", DoubleValue(startTime));
  SetAttribute ("ModelStepSize", DoubleValue(commStepSize));
  SetAttribute ("LoggingOn", BooleanValue (loggingOn));
  SetAttribute ("InitCallback", CallbackValue (initCallback));
  SetAttribute ("DoStepCallback", CallbackValue (doStepCallback));
}

template <class FmuDeviceType>
void 
FmuDeviceHelper<FmuDeviceType>::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

template <class FmuDeviceType>
ApplicationContainer
FmuDeviceHelper<FmuDeviceType>::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

template <class FmuDeviceType>
ApplicationContainer
FmuDeviceHelper<FmuDeviceType>::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
      apps.Add (InstallPriv (*i));
  }
  return apps;
}

template <class FmuDeviceType>
Ptr<Application>
FmuDeviceHelper<FmuDeviceType>::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<FmuDeviceType> ();
  node->AddApplication (app);
  return app;
}

} // namespace ns3

#endif /* FMU_DEVICE_HELPER_H */
