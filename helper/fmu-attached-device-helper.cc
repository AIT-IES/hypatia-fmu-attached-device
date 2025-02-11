#include "fmu-attached-device-helper.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "ns3/names.h"

using namespace std;

namespace ns3 {

FmuAttachedDeviceHelper::FmuAttachedDeviceHelper (
  uint16_t port, uint16_t nodeId, const string& modelIdentifier, 
  const double& startTime, const double& commStepSize, bool loggingOn
) {
  m_factory.SetTypeId (FmuAttachedDevice::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("NodeId", UintegerValue (nodeId));
  SetAttribute ("ModelIdentifier", StringValue (modelIdentifier));
  SetAttribute ("ModelStartTime", DoubleValue(startTime));
  SetAttribute ("ModelStepSize", DoubleValue(commStepSize));
  SetAttribute ("LoggingOn", BooleanValue (loggingOn));
}

FmuAttachedDeviceHelper::FmuAttachedDeviceHelper (
  uint16_t port, uint16_t nodeId, const string& modelIdentifier, 
  const double& startTime, const double& commStepSize, bool loggingOn, 
  DoStepCallbackType doStepCallback
) {
  m_factory.SetTypeId (FmuAttachedDevice::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("NodeId", UintegerValue (nodeId));
  SetAttribute ("ModelIdentifier", StringValue (modelIdentifier));
  SetAttribute ("ModelStartTime", DoubleValue(startTime));
  SetAttribute ("ModelStepSize", DoubleValue(commStepSize));
  SetAttribute ("LoggingOn", BooleanValue (loggingOn));
  SetAttribute ("DoStepCallback", CallbackValue (doStepCallback));
}

FmuAttachedDeviceHelper::FmuAttachedDeviceHelper (
  uint16_t port, uint16_t nodeId, const string& modelIdentifier, 
  const double& startTime, const double& commStepSize, bool loggingOn, 
  InitCallbackType initCallback, DoStepCallbackType doStepCallback
) {
  m_factory.SetTypeId (FmuAttachedDevice::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("NodeId", UintegerValue (nodeId));
  SetAttribute ("ModelIdentifier", StringValue (modelIdentifier));
  SetAttribute ("ModelStartTime", DoubleValue(startTime));
  SetAttribute ("ModelStepSize", DoubleValue(commStepSize));
  SetAttribute ("LoggingOn", BooleanValue (loggingOn));
  SetAttribute ("InitCallback", CallbackValue (initCallback));
  SetAttribute ("DoStepCallback", CallbackValue (doStepCallback));
}

void 
FmuAttachedDeviceHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
FmuAttachedDeviceHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
FmuAttachedDeviceHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
      apps.Add (InstallPriv (*i));
  }
  return apps;
}

Ptr<Application>
FmuAttachedDeviceHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<FmuAttachedDevice> ();
  node->AddApplication (app);
  return app;
}

} // namespace ns3
