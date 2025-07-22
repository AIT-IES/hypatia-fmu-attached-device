#include "device-client-helper.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

#include <string>

namespace ns3 {

DeviceClientHelper::DeviceClientHelper (
  Address address, uint16_t port, uint64_t from_node_id, uint64_t to_node_id, 
  MessageSendCallbackType send_callback, MessageReceiveCallbackType receive_callback,
  Time processing_time_const, Time processing_time_mean, Time processing_time_std_dev)
{
  m_factory.SetTypeId (DeviceClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
  SetAttribute ("FromNodeId", UintegerValue (from_node_id));
  SetAttribute ("ToNodeId", UintegerValue (to_node_id));
  SetAttribute ("MsgSendCallback", CallbackValue (send_callback));
  SetAttribute ("MsgReceiveCallback", CallbackValue (receive_callback));
  SetAttribute ("ProcessingTimeConstant", TimeValue (processing_time_const));
  SetAttribute ("ProcessingTimeMean", TimeValue (processing_time_mean));
  SetAttribute ("ProcessingTimeStdDev", TimeValue (processing_time_std_dev));
}

void 
DeviceClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
DeviceClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
DeviceClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<DeviceClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
