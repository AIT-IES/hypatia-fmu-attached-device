#ifndef FMU_SHARED_DEVICE_H
#define FMU_SHARED_DEVICE_H

#include "fmu-attached-device.h"

#include <map>

namespace ns3 {

class FmuSharedDevice : public FmuAttachedDevice
{
public:

  static TypeId GetTypeId (void);
  FmuSharedDevice () {}
  virtual ~FmuSharedDevice () {}

private:
  virtual void initFmu();
  virtual Payload stepFmu(const std::string& payload, uint32_t payloadId, bool isReply, const double& t);

  std::string m_sharedFmuInstanceName;

  typedef std::map<std::string, Ptr<RefFMU> > SharedFmuCollection;
  static SharedFmuCollection m_sharedFmuCollection;
};

} // namespace ns3

#endif /* FMU_SHARED_DEVICE_H */
