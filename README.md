# FMU-attached Devices for Hypatia

## About

This package provides [ns-3](https://www.nsnam.org/) application layer models that use a [Functional Mock-up Unit](https://fmi-standard.org/) (FMU) for Co-Simulation (FMI 2.0) to compute its internal state.
This package is primarily intended to work with the ns-3 simulator provided by the [Hpyatia](https://github.com/snkas/hypatia) simulator.
However, it can also be used with standard ns-3 distrubtions.

Examples for the usage are available [here](https://github.com/AIT-IES/hypatia-fmu-attached-device-demo.git).

## Quick start (Ubuntu 20.04)

1. Start from a [clean Hypatia installation](https://github.com/snkas/hypatia?tab=readme-ov-file#getting-started).
2. Install dependencies:
   ``` bash
   sudo apt install cmake
   ```
3. Switch to the folder containing Hypatia's ns-3 installation (replace `<hypatia_root_dir>` with Hypatia's root directory):
   ``` bash
   cd <hypatia_root_dir>/ns3-sat-sim/simulator
   ```
4. Checkout this module:
   ``` bash
   git clone https://github.com/AIT-IES/hypatia-fmu-attached-device.git ./contrib/fmu-attached-device
   ```
5. Configure and re-build ns-3 with the new module:
   ``` bash
   ./waf configure --build-profile=debug --enable-mpi --enable-examples --enable-tests --enable-gcov --out=build/debug_all
   ./waf -j4
   ```

## Usage

Examples for the usage are available [here](https://github.com/AIT-IES/hypatia-fmu-attached-device-demo.git).

### Class `FMUAttachedDevice`

This class implements an application layer model that uses an FMU to compute its internal state.
The interaction of the device with the FMU (initialization and simulation of the model) can be defined via callbacks.
When receiving a message, the FMU will be used to determine the device's current state and the device will also return a message.
The content of the return message is also determined via the callback function for simulating the FMU.
The sending of the return message is delayed by a process delay (randomized using a gamma distribution).

Attributes:

+ *Port*: port on which we listen for incoming packets (UintegerValue)
+ *NodeId*: node identifier (UintegerValue)
+ *ModelIdentifier*: FMU model identifier (StringValue)
+ *ModelStepSize*: Set the communication step size for the FMU in seconds (DoubleValue)
+ *ModelStartTime*: Set the start time for the FMU in seconds (DoubleValue)
+ *LoggingOn*: Turn on logging for FMU (BooleanValue)
+ *InitCallback*: Callback for instantiating and initializing the FMU model (CallbackValue)
+ *DoStepCallback*: Callback for performing a simulation step and returning a payload message (CallbackValue)
+ *ResultsWrite*: Flag to indicate if results file should be written (BooleanValue)
+ *ResultsWritePeriodInS*: Time period to write values to results file (DoubleValue)
+ *ResultsFilename*: Name of results file (StringValue)
+ *ResultsVariableNamesList*: List of names of variables whose values should be written to the results file (StringValue)
+ *ProcessingTimeMean*: Average processing time (TimeValue
+ *ProcessingTimeStdDev*: Standard deviation of processing time (TimeValue)

Class `FmuAttachedDeviceHelper` implements a helper API for class `FMUAttachedDevice`.

### Class `FmuSharedDevice`

This class implements application layer models that uses a common shared FMU to compute their internal states.
The usage and function is analogous to class `FMUAttachedDevice`.

Class `FmuSharedDevice` has the same parameters as class `FMUAttachedDevice`.
In addition, it has the following parameter:
+ *SharedFmuInstanceName*: Common name of the shared FMU instance (StringValue)

Class `FmuSharedDeviceHelper` implements a helper API for class `FMUSharedDevice`.

### Class `DeviceClient`

A simple client that sends/receives messages to/from FMU-attached devices.
The behavior of this client (sending and receiving) can be defined via callbacks.
The client sends messages in regular intervals, randomized by a processing delay (subject to a gamma distribution).

Attributes:

+ *Interval*: The time to wait between packets (TimeValue)
+ *RemoteAddress*: The destination Address of the outbound packets (AddressValue)
+ *RemotePort*: The destination port of the outbound packets (UintegerValue)
+ *FromNodeId*: From node identifier (UintegerValue)
+ *ToNodeId*: To node identifier (UintegerValue)
+ *MsgSendCallback*: Callback for sending a payload message (CallbackValue)
+ *MsgReceiveCallback*: Callback for receiving a payload message (CallbackValue)
+ *ProcessingTimeMean*: Average processing time (TimeValue
+ *ProcessingTimeStdDev*: Standard deviation of processing time (TimeValue)

Class `DeviceClientHelper` implements a helper API for class `DeviceClient`.

### Class `FmuAttachedDeviceFactory`

This class eases the deployment FMU-attached devices in a simulation setup.
It uses the Hypatia's `BasicSimulation` class for defining a simulation setup via a config file (`config_ns3.properties`) and applies it to an ns-3 topology.

In the simulation config file (`config_ns3.properties`), the following properties are expected:

+ *enable_fmu_attached_devices*: enable the use of this factory (boolean)
+ *fmu_config_files*: mapping of node IDs to FMU config file names (map); for each node ID (which has to correspond to a node in the ns-3 topology), an FMU-attached device according to the specified FMU config file will be created

Example simulation config file snippet:
``` properties
enable_fmu_attached_devices=true
fmu_config_files=map(1252:simple-fmu-attached-device.txt)
```

In the FMU config files, the following properties are expected:

+ *model_identifier*: FMU model identifier (string)
+ *fmu_dir*: path (relative to run directory or absolute) to the directory containing the extracted FMU (string)
+ *processing_time_mean_ns*: average of processing time of FMU-attached device in nanoseconds (double)
+ *processing_time_std_dev_ns*: standard deviation of processing time of FMU-attached device in nanoseconds (double)
+ *start_time_in_s*: FMU model start time in seconds (double)
+ *comm_step_size_in_s*: FMU model communication step size in seconds (double)
+ *logging_on*: turn on/off the logger of the FMU model (boolean)
+ *fmu_res_write*: turn on/off the writing of FMU model results (boolean)
+ *fmu_res_write_period_in_s*: period in seconds for writing of FMU model results (double)
+ *fmu_res_filename*: file name for FMU model results
+ *fmu_res_varnames*: names of FMU model variables to be written to results file (list of strings)

Example FMU config file snippet:
``` properties
model_identifier=integrate
fmu_dir_uri=file:///path/to/hypatia/dev/extracted_fmu
processing_time_mean_ns=100000
processing_time_std_dev_ns=20000
start_time_in_s=0.
comm_step_size_in_s=1e-4
logging_on=false
fmu_res_write=true
fmu_res_write_period_in_s=0.5
fmu_res_filename=simple-fmu-attached-device.csv
fmu_res_varnames=list(x,k)
```

### Class `FmuSharedDeviceFactory`

This class eases the deployment devices attached to the same FMU in a simulation setup.
The usage and function is similar to class `FMUAttachedDeviceFactory`.

In the simulation config file (`config_ns3.properties`), the following properties are expected:

+ *enable_fmu_shared_devices*: enable the use of this factory (boolean)
+ *fmu_config_files*: mapping of a set of node IDs to FMU config file names (map); the set of node IDs is referred to by name and expected to be present in the configuration; for each node ID in the set, a device sharing the same attached FMU according to the specified FMU config file will be created

Example simulation config file snippet:
``` properties
enable_fmu_shared_devices=true
fmu_config_files=map(example_shared_devices:shared-fmu.properties)
example_shared_devices=set(1251,1252)
```

In the FMU config files, class `FmuSharedDeviceFactory` expects the same properties as class `FMUAttachedDeviceFactory`.
In addition, it expects the following property:
+ *shared_instance_name*: Common name of the shared FMU instance (string)


### Class `DeviceClientFactory`

This class eases the deployment of device clients in a simulation setup.
It uses the Hypatia's `BasicSimulation` class for defining a simulation setup via a config file (`config_ns3.properties`) and applies it to an ns-3 topology.

In the simulation config file (`config_ns3.properties`), the following properties are expected:

+ *enable_device_clients*: enable the use of this factory (boolean)
+ *send_devices_interval_ns*: interval in nanoseconds for sending requests from clients (integer)
+ *send_devices_endpoint_pairs*: set of node IDs defining pairs of clients and devices (set of strings of the form *"[client-id]->[device-id]"*)
+ *send_devices_processing_time_mean_ns*: average of processing time of clients in nanoseconds (double)
+ *send_devices_processing_time_std_dev_ns*: standard deviation of processing time of clients in nanoseconds (double)

Example simulation config file snippet:
``` properties
enable_device_clients=true
send_devices_interval_ns=100000000
send_devices_endpoint_pairs=set(1170->1252)
send_devices_processing_time_mean_ns=100000
send_devices_processing_time_std_dev_ns=20000
```
