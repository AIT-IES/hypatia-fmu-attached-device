#ifndef FMU_ATTACHED_DEVICE_H
#define FMU_ATTACHED_DEVICE_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/fmu-util.h"
#include "ns3/processing-time.h"
#include "ns3/ptr.h"
#include "ns3/seq-ts-header.h"
#include "ns3/traced-callback.h"

#include <string>
#include <vector>

namespace ns3 {

class Socket;
struct SendReplyContext;

class FmuAttachedDevice : public Application 
{
public:
  typedef Callback<void, Ptr<RefFMU>, uint64_t, const std::string&, const double&> InitCallbackType;
  typedef Callback<std::string, Ptr<RefFMU>, uint64_t, const std::string&, const double&, const double&> DoStepCallbackType;

  static TypeId GetTypeId (void);
  FmuAttachedDevice ();
  virtual ~FmuAttachedDevice ();

  static void defaultInitCallbackImpl(Ptr<RefFMU> fmu, uint64_t nodeId, const std::string& modelIdentifier, const double& startTime);
  static std::string defaultDoStepCallbackImpl(Ptr<RefFMU> fmu, uint64_t nodeId, const std::string& payload, const double& time, const double& commStepSize);

protected:

  virtual void DoDispose (void);
  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void HandleRead (Ptr<Socket> socket);
  void Send(Ptr<SendReplyContext> reply);
  void WriteData (void);

  uint16_t m_port;      //!< Port on which we listen for incoming packets.
  uint64_t m_nodeId;      //!< Node identifier.

  Ptr<Socket> m_socket; //!< IPv4 Socket
  Address m_local;      //!< local multicast address

  Time m_processingTimeConstant; //!< Constant term of processing time.
  Time m_processingTimeMean; //!< Average of stochastic term of processing time.
  Time m_processingTimeStdDev; //!< Standard deviation of stochastic term of processing time.
  Ptr<ProcessingTime> m_processingTime;

  virtual void initFmu();
  virtual std::string stepFmu(const std::string& payload, const double& t);

  std::string m_modelIdentifier;
  bool m_loggingOn;
  double m_commStepSizeInS;
  double m_startTimeInS;
  Ptr<RefFMU> m_fmu;

  InitCallbackType m_initCallback;
  DoStepCallbackType m_doStepCallback;

  EventId m_writeDataEvent; //!< Event to write FMU model data.
  EventId m_sendEvent; //!< Event to send back data.

  bool m_resWrite; 
  double m_resWritePeriodInS;
  std::string m_resFilename; 
  std::string m_resVarnamesList;
  std::vector<std::string> m_resVarnames;
};

} // namespace ns3

#endif /* FMU_ATTACHED_DEVICE_H */
