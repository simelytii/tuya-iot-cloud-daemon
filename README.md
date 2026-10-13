# Tuya IoT Monitoring Daemon

A Linux system-monitoring daemon written in C that collects system telemetry and reports it to the Tuya IoT platform using the Tuya IoT Core SDK.

The project explores Linux daemon development, system information collection, JSON telemetry generation, MQTT-based cloud communication, signal handling, and Makefile-based builds.

## Features

- **System monitoring** — collects total and free RAM, system uptime, and CPU usage.
- **Network monitoring** — collects network interface names, IPv4 addresses, netmasks, and transmitted/received traffic.
- **JSON telemetry** — structures measurements into JSON objects compatible with the configured Tuya telemetry properties.
- **Tuya IoT integration** — initializes the Tuya client, connects to the cloud, and reports telemetry periodically.
- **Daemon mode** — supports background execution using the `-D` option.
- **Signal handling** — handles `SIGINT`, `SIGTERM`, and `SIGQUIT` to initiate an orderly shutdown.
- **Modular C code** — separates command-line parsing, system information, network information, telemetry generation, daemon management, and Tuya client integration.

## Architecture

The application is organized into several functional modules:

| Module | Responsibility |
|---|---|
| `main.c` | Application entry point, initialization, main loop, and shutdown |
| `cli.c` | Command-line argument parsing |
| `daemon.c` | Daemonization |
| `system_info.c` | System memory and uptime information |
| `cpu_info.c` | CPU usage collection |
| `network_info.c` | Network interface information and traffic counters |
| `telemetry.c` | Telemetry collection and reporting flow |
| `telemetry_json.c` | JSON representation of telemetry |
| `tuya_client.c` | Tuya IoT client initialization and communication |
| `action.c` | Application action handling |

Public interfaces are declared in the `include/` directory. The Tuya IoT Core SDK is included as a dependency under `tuya-iot-core-sdk/`.

## Telemetry

The generated telemetry includes:

| Property | Description |
|---|---|
| `total_ram` | Total RAM in MiB |
| `free_ram` | Free RAM in MiB |
| `system_uptime` | System uptime |
| `cpu_usage` | CPU usage |
| `interfaces` | Array containing network interface information |

Each network interface entry contains:

- `name` — interface name
- `ip_addr` — IPv4 address
- `netmask` — network mask
- `tx_mb` — transmitted traffic in MiB
- `rx_mb` — received traffic in MiB

RAM and CPU values are rounded to two decimal places. Network traffic counters are converted to whole MiB values.

The corresponding properties must be configured in the Tuya IoT platform to match the telemetry structure used by the application.

## Requirements

- Linux development environment
- C compiler and GNU Make
- CMake
- Development libraries and headers required by the Tuya IoT Core SDK
- A Tuya IoT platform device with the appropriate device credentials and product configuration

The SDK build and application link against the libraries defined in the project's `Makefile`, including the Tuya SDK libraries and `cJSON`.

## Build

Clone the repository. The Tuya IoT Core SDK source is included in the repository under `tuya-iot-core-sdk/`.

```bash
git clone https://github.com/simelytii/tuya-daemon.git
cd tuya-daemon
make
```

The Makefile builds the Tuya IoT Core SDK with CMake and then compiles the daemon.

To remove the application executable and its object files:

```bash
make clean
```

The SDK build directory is separate from the application's object files. If you need a completely fresh SDK build, remove its build directory before running `make` again:

```bash
rm -rf tuya-iot-core-sdk/build
make
```

## Configuration and Usage

Obtain the device ID, device secret, and product ID from your Tuya IoT platform project. Do not commit real credentials to source control.

Run the application in the foreground:

```bash
./tuya-daemon \
  --device-id "<DEVICE_ID>" \
  --device-secret "<DEVICE_SECRET>" \
  --product-id "<PRODUCT_ID>"
```

Run in daemon mode:

```bash
./tuya-daemon \
  --device-id "<DEVICE_ID>" \
  --device-secret "<DEVICE_SECRET>" \
  --product-id "<PRODUCT_ID>" \
  --daemon
```

Short options are also available: `-d` for device ID, `-s` for device secret, `-p` for product ID, and `-D` for daemon mode.

Display command-line help:

```bash
./tuya-daemon --help
```

The daemon reports telemetry in a loop with a 10-second sleep between reporting iterations. Actual cloud reporting depends on valid credentials, network connectivity, the SDK, and the corresponding Tuya device properties.

## Teltonika RutOS Adaptation

A separate development branch adapts the daemon for the Teltonika RutOS/OpenWrt package environment.

The adaptation includes:

- OpenWrt-style package definitions for the daemon and Tuya IoT Core SDK
- A `procd` init script for service lifecycle management
- UCI configuration under `/etc/config/tuya-daemon`
- `ubus` integration for daemon status and on-demand reporting
- A separate RutOS API package with access-control and VUCI route definitions
- Cross-compilation and packaging for the target router architecture

The service is disabled by default in the supplied configuration. Device credentials must be configured on the target device before enabling it.

The RutOS adaptation is maintained in a separate repository and includes router-specific package definitions and service integration. Its source repository may require authorized access.

## Repository Layout

```text
.
├── include/              # Application header files
├── src/                  # Daemon implementation
├── tuya-iot-core-sdk/    # Tuya IoT Core SDK
├── Makefile              # Build rules
└── README.md
```

## Security Notes

- Never commit real Tuya device secrets or credentials.
- Use placeholders in examples and documentation.
- Restrict access to configuration files containing credentials.
- Review the security implications of running the daemon with elevated privileges on embedded devices.

## Scope

This project demonstrates embedded Linux application development and IoT integration. The RutOS package adaptation extends the standalone application with router-specific service management, configuration, and control interfaces.

Hardware-specific deployment and cloud connectivity should be validated on the intended target device and Tuya project configuration.

## License

Check the licensing terms of the application and the bundled Tuya IoT Core SDK before redistribution.
