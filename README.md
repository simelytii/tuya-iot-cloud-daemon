# Tuya IoT Cloud Daemon for RutOS

A C-based Linux daemon adapted for Teltonika RutOS routers. It collects system telemetry and reports it to Tuya IoT Cloud.

## Features

- System memory and uptime monitoring.
- CPU usage and network interface telemetry.
- Tuya IoT Core SDK integration.
- RutOS service management using procd.
- UCI-based configuration.
- Local daemon operations through ubus.
- VUCI API integration.
- Separate packages for the daemon, SDK, and API.

## Package Structure

- `package/libs/libtuyasdk/` — Tuya IoT Core SDK package.
- `package/tuya-daemon/` — daemon source, package definition, init script, and configuration.
- `package/tuya-daemon-api/` — RutOS Lua API, ACL, and routing integration.

## Configuration

The configuration file is `/etc/config/tuya-daemon`.

The service is disabled by default. Configure valid Tuya credentials before enabling it.

Example configuration:

    config tuya_daemon 'main'
        option enabled '0'
        option device_id 'YOUR_DEVICE_ID'
        option device_secret 'YOUR_DEVICE_SECRET'
        option product_id 'YOUR_PRODUCT_ID'

Never commit real device credentials to a public repository.

## Build Environment

The project was developed for a Teltonika RUTX08 target using the RutOS/OpenWrt build system.

- Target: `ipq40xx`
- Architecture: `arm_cortex-a7_neon-vfpv4`
- Language: C
- Service manager: procd
- Configuration: UCI
- Local IPC: ubus
- Router API integration: VUCI

## Building

Use a compatible RutOS/OpenWrt source tree containing these package directories.

    make menuconfig
    make package/libs/libtuyasdk/compile V=s
    make package/tuya-daemon/compile V=s
    make package/tuya-daemon-api/compile V=s

Select the required packages and dependencies in the build configuration first. Generated `.ipk` files are placed in the target's package output directory.

## Running and Diagnostics

After installing the packages on a compatible test router, configure valid credentials and enable the service:

    uci set tuya-daemon.main.enabled='1'
    uci commit tuya-daemon
    /etc/init.d/tuya-daemon enable
    /etc/init.d/tuya-daemon start

Inspect the service logs with:

    logread -e tuya-daemon

Verify the available ubus operations against the installed build.

## Security

- Keep the service disabled until valid credentials are configured.
- Do not publish device secrets.
- Review API access-control rules before deployment.
- Test on a development router before using the integration in production.

## Project Scope

This repository contains the RutOS-specific packaging and integration for the daemon. It is separate from the standalone Linux daemon project because RutOS requires additional package definitions, service management, UCI configuration, and router API integration.

## Third-Party Components

The project uses the Tuya IoT Core SDK. Review the upstream SDK license and applicable project licensing terms before redistribution.
