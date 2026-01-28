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
+ *send_data*: enable sending of data to client devices; turned off by default (boolean)
+ *send_data_interval_s*: interval in s for sending data (double)
+ *send_data_endpoint*: client endpoint node ID for sending data (integer)

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

In the FMU config files, class `FmuSharedDeviceFactory` expects the same properties as class `FMUAttachedDeviceFactory` (except property *send_data_endpoint*, which is not supported).
In addition, it expects the following property:
+ *shared_instance_name*: common name of the shared FMU instance (string)
+ *send_data_endpoints*: list of client endpoint node IDs for sending data (set of strings of the form *"[device-id]->[client-id]"*)

### Class `DeviceClientFactory`

This class eases the deployment of device clients in a simulation setup.
It uses the Hypatia's `BasicSimulation` class for defining a simulation setup via a config file (`config_ns3.properties`) and applies it to an ns-3 topology.

In the simulation config file (`config_ns3.properties`), the following properties are expected:

+ *enable_device_clients*: enable the use of this factory (boolean)
+ *send_devices_interval_ns*: interval in nanoseconds for sending requests from clients (integer)
+ *send_devices_endpoint_pairs*: set of node IDs defining pairs of clients and devices (set of strings of the form *"[client-id]->[device-id]"*)
+ *send_devices_processing_time_mean_ns*: average of processing time of clients in nanoseconds (double)
+ *send_devices_processing_time_std_dev_ns*: standard deviation of processing time of clients in nanoseconds (double)

Clients can also be installed in *listen-only mode*:

+ *send_devices*: when setting this to false, the client will be deployed in listen-only mode (boolean)
+ *devices_receive_endpoints*: when in listen-only mode, this determines the node IDs on which the clients will be installed; all other properties (*send_devices_interval_ns*, *send_devices_endpoint_pairs*, etc.) will be ignored (set of strings)

Example simulation config file snippet:
``` properties
enable_device_clients=true
send_devices_interval_ns=100000000
send_devices_endpoint_pairs=set(1170->1252)
send_devices_processing_time_mean_ns=100000
send_devices_processing_time_std_dev_ns=20000
```

## Funding acknowledgement

<svg align="left" style="margin-right: 10px" height="64.195998" viewBox="0 0 531.53333 213.98666" width="159.459999" xml:space="preserve" xmlns="http://www.w3.org/2000/svg"><g transform="matrix(.13333333 0 0 -.13333333 0 213.98667)"><path d="m1630.4 1073.84c-34.18 10.43-71.65 16.71-116.19 16.29-55.17-.67-109.49-10.92-165.63-28.52-19.18-4.27-37.81-6.97-55.85-7.56-41.34.02-70.71 12.45-86.07 33.54-18.92 26.47-16.8 66.7 9.87 113.18 10.52 17.75 20.94 30.73 34.57 45.22l-.52.02c38.58 36.09 69.81 74.51 94.24 116.81 83.31 123.13 44.17 237.65-111.1 239.6-155.42-3.36-304.931-79.89-421.865-230.2-121.066-150.39-83.042-292.79 69.54-296.27 39.742-.51 82.371 7.93 128.385 23.69 18.13 4.28 32.51 6.56 48.41 6.68 29.16-.25 52.26-8.27 67.82-21.93 29-25.67 31-71.32-1.71-125.593-8.85-14.602-18.16-25.504-29.62-37.918-50.41-42.688-92.96-88.227-126.629-139.281-30.922-47.442-48.485-90.45-54.852-131.102-.508 1.082-.523.535-.496 1.602-5.808-19.465-11.933-30.442-23.488-47.09-41.856-59.887-71.863-69.156-136.059-102.18-25.676-9.168-86.441-30.488-107.121-30.476-19.601.507-22.25.589-39.547 7.918-25.515 17.652-57.586 27.48-98.902 29.101-154.738 2.449-319.27-55.531-449.5743-213.801-102.7226-135.742-71.5469-295.5582812 113.7113-295.308281 36.683-3.671879 297.14 31.187481 419.73 209.508281 34.606 46.269 54.27 88.699 62.727 128.23 6.215 14.699 11.785 24.621 21.129 37.621 29.679 40.039 64.613 56.07 110.867 81.359 46.254 25.282 95.191 51.211 141.859 52.122 22.25-.59 35.969-3.614 50.133-9.274 33.932-19.43 78.182-30.68 136.462-31.668 111.75 1.649 276.83 8.988 471.26 159.395.54.508 102.7 87.304 136.03 155.508 23.27 38.078 35.82 72.218 40.42 106.015.6 2.633 1.77 6.844 2.99 13.176 6.23 35.355 8.57 124.356-134.93 171.586" fill="#ed1639"/><g fill="#231f20"><path d="m2150.08 647.137v17.812h-115.85v-186.597h19.93v82.289h81.77v17.82h-81.77v68.676zm130.53-104.309c0 19.91-2.62 38.004-15.73 51.363-8.92 8.899-21.22 14.672-36.96 14.672-15.72 0-28.03-5.773-36.96-14.672-13.09-13.359-15.72-31.453-15.72-51.363 0-19.93 2.63-38.019 15.72-51.379 8.93-8.898 21.24-14.679 36.96-14.679 15.74 0 28.04 5.781 36.96 14.679 13.11 13.36 15.73 31.449 15.73 51.379m-18.88 0c0-14.43-.78-30.406-10.21-39.848-6.04-6.031-14.43-9.429-23.6-9.429s-17.29 3.398-23.32 9.429c-9.44 9.442-10.49 25.418-10.49 39.848 0 14.402 1.05 30.391 10.49 39.82 6.03 6.043 14.15 9.442 23.32 9.442s17.56-3.399 23.6-9.442c9.43-9.429 10.21-25.418 10.21-39.82m145.62 53.973c-9.16 9.183-18.6 12.062-30.92 12.062-14.94 0-29.09-6.543-36.17-17.293v15.723h-18.87v-128.941h18.87v79.148c0 19.66 12.06 34.59 30.93 34.59 9.96 0 15.2-2.352 22.28-9.442zm123.16-80.723c0 24.113-15.45 32.762-38.01 34.863l-20.7 1.84c-16.25 1.309-22.54 7.86-22.54 18.867 0 13.102 9.96 21.243 28.84 21.243 13.36 0 25.16-3.153 34.33-10.243l12.32 12.332c-11.53 9.43-28.05 13.883-46.4 13.883-27.52 0-47.43-14.152-47.43-37.734 0-21.238 13.37-32.508 38.53-34.609l21.23-1.829c14.93-1.312 21.49-7.601 21.49-18.859 0-15.223-13.1-22.812-34.33-22.812-16 0-29.88 4.191-40.11 14.949l-12.57-12.59c14.14-13.641 31.18-18.609 52.94-18.609 31.18 0 52.41 14.41 52.41 39.308m131.86-20.18-12.84 12.321c-9.69-10.75-17.3-14.668-29.61-14.668-12.59 0-23.07 4.969-29.89 14.668-6.01 8.39-8.39 18.351-8.39 34.609 0 16.242 2.38 26.203 8.39 34.594 6.82 9.699 17.3 14.668 29.89 14.668 12.31 0 19.92-3.668 29.61-14.41l12.84 12.058c-13.37 14.41-24.63 19.125-42.45 19.125-32.5 0-57.14-22.011-57.14-66.035 0-44.039 24.64-66.058 57.14-66.058 17.82 0 29.08 4.718 42.45 19.128m137.48-17.546v82.82c0 29.09-17.32 47.691-46.41 47.691-14.4 0-26.73-4.972-36.16-15.722v71.808h-18.87v-186.597h18.87v79.668c0 22.289 12.85 34.07 32.24 34.07s31.44-11.531 31.44-34.07v-79.668zm145.46 0v128.941h-18.87v-79.414c0-22.551-12.85-34.328-32.25-34.328-19.38 0-31.44 11.519-31.44 34.328v79.414h-18.88v-82.293c0-14.93 3.94-27.262 13.11-36.172 7.87-7.859 19.4-12.058 33.28-12.058 14.42 0 27.26 5.5 36.43 15.98v-14.398zm147.82 0v82.546c0 14.954-4.2 27-13.36 35.903-7.88 7.859-19.14 12.062-33.04 12.062-14.41 0-26.99-5.242-36.16-15.722v14.152h-18.88v-128.941h18.88v79.398c0 22.559 12.58 34.34 31.97 34.34 19.4 0 31.72-11.531 31.72-34.34v-79.398zm141.74-3.942v132.883h-18.62v-15.203c-10.47 13.629-22.01 16.773-36.16 16.773-13.09 0-24.63-4.453-31.45-11.261-12.83-12.852-15.73-32.762-15.73-53.731 0-20.973 2.9-40.891 15.73-53.742 6.82-6.809 18.08-11.527 31.19-11.527 13.89 0 25.69 3.418 36.17 16.777v-20.18c0-22.019-10.48-39.59-35.38-39.59-14.94 0-21.48 4.461-30.94 12.852l-12.3-12.063c13.62-12.32 24.37-17.289 43.77-17.289 33.81 0 53.72 23.332 53.72 55.301m-18.87 69.461c0-24.121-3.94-48.23-31.98-48.23-28.03 0-32.24 24.109-32.24 48.23 0 24.109 4.21 48.219 32.24 48.219 28.04 0 31.98-24.11 31.98-48.219m299.83 63.422h-20.43l-29.36-103.531-34.07 103.531h-16.24l-33.82-103.531-29.62 103.531h-20.45l40.89-128.941h17.57l33.54 100.109 33.81-100.109h17.57zm51.88 57.93h-21.23v-21.231h21.23zm-1.3-57.93h-18.87v-128.953h18.87zm132.22-10.492c-9.18 9.183-18.62 12.062-30.93 12.062-14.94 0-29.09-6.543-36.17-17.293v15.723h-18.87v-128.941h18.87v79.148c0 19.66 12.05 34.59 30.92 34.59 9.97 0 15.21-2.352 22.28-9.442zm138.49-118.449-51.37 79.668 43.77 49.273h-23.59l-58.19-67.102v124.758h-18.87v-186.597h18.87v37.207l25.15 28.839 40.9-66.046zm86.6 0v16.25h-9.97c-12.05 0-17.56 7.07-17.56 18.859v78.629h27.53v14.422h-27.53v40.371h-18.87v-40.371h-16.24v-14.422h16.24v-79.149c0-19.132 11-34.589 33.03-34.589zm62.76 24.628h-24.63v-24.628h24.63z"/><path d="m2541.88 1464.07v134.33h-508.06v-771.283h150.58v313.053h304.39v134.33h-304.39v189.57zm676.94 0v134.33h-508.06v-771.283h150.57v313.053h304.41v134.33h-304.41v189.57zm701.97-320.65v112.67h-291.42v-125.67h141.91v-29.24c0-40.09-9.74-74.75-34.66-102.918-24.92-27.09-61.75-43.332-107.25-43.332-41.16 0-74.75 15.168-96.42 40.086-29.23 32.494-36.81 69.324-36.81 217.734 0 148.4 7.58 184.16 36.81 216.66 21.67 24.91 55.26 41.16 96.42 41.16 76.91 0 121.34-40.09 138.66-112.66h151.66c-20.57 130-111.57 246.99-290.32 246.99-86.66 0-153.82-30.34-207.98-84.5-78.01-78-75.83-174.41-75.83-307.65s-2.18-229.656 75.83-307.656c54.16-54.16 123.49-84.492 207.98-84.492 82.33 0 156.01 23.832 217.74 87.746 54.16 56.328 73.68 123.502 73.68 235.072"/></g></g></svg> This work has been funded by the [Austrian Research Promotion Agency FFG](https://www.ffg.at) as part of the **STARS** project under grant agreement FO999914870.
