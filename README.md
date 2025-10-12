# ESP32 Wake On Lan Trigger

My home rig is connected over WiFi to my home network due to living in a
rental appartment. It is also not always on as I do not appreciate the
faint sound of my fans.

As a result, I often want to turn it on when I am either not next to it, or
when I am working from a café (via Tailscale back to the home network). The
typical solution here is Wake On Lan, which is unfortunately not available
over WiFi.

This small ESP32 project fixes this, by plugging the [ESP32-S3-ETH](https://www.waveshare.com/wiki/ESP32-S3-ETH)
over Ethernet to my rig, and connecting it over WiFi to my home network, I
now can trigger the emission of Wake On Lan pakcets via simple, over WiFi,
HTTP calls.

Once deployed, you can easily wake up your rig via a simple HTTP POST request:
```sh
# If you are fancy and use HTTPie
http POST http://esp32-wol:80/wol

# If you are a curl die hard
curl -X POST http://esp32-wol:80/wol
```

Prefer a visual interface? Browse to `http://esp32-wol/` and click the **Wake Up**
button. The embedded web UI calls the same endpoint, shows the current status,
and works from mobile browsers too.

## Prerequisites

- ESP-IDF v5.0 or newer (install and export the environment as documented by Espressif)
- Waveshare ESP32-S3 ETH board (integrates an ESP32-S3 module with a W5500 Ethernet PHY)
- USB-to-UART driver installed on the host

After installing ESP-IDF, remember to export the tools in every shell:

```bash
. ${IDF_PATH}/export.sh
```

The project is already configured for the ESP32-S3 target and the Waveshare
Ethernet pinout via `sdkconfig.defaults`, so you do not need to run
`idf.py set-target`. You however need to configure your WiFi credentials via
`idf.py menuconfig` (see below).

When the ESP32 is flashed, you can then connect it over Ethernet to the target
computer.

> Note that this assumes the configuration of your computer or server to support
> Wake On Lan, which is typically done via a mix of Bios/UEFI settings (refer
> to your motherboard documentation) and system settings.

## Quick Start Commands

Run the following from the repository root.

| Purpose | Command | Notes |
| --- | --- | --- |
| Configure project options (optional) | `idf.py menuconfig` | Only needed if you want to tweak defaults from `sdkconfig.defaults`. |
| Build the firmware | `idf.py build` | Outputs the application binary under `build/`. |
| Flash over USB | `idf.py -p /dev/ttyACM0 flash` | Replace the port with your OS-specific device path. |
| Flash and immediately monitor | `idf.py -p /dev/ttyACM0 flash monitor` | Press <kbd>Ctrl</kbd>+<kbd>]</kbd> to exit the monitor. |
| Monitor only | `idf.py -p /dev/ttyACM0 monitor` | Useful when the device is already flashed. |

### Wi-Fi Credentials

Run `idf.py menuconfig`, open `Esp32 WoL Configuration`, and update `WiFi SSID` and `WiFi Password`. The values persist in `sdkconfig`, so rerun menuconfig if you need to change them later.

### Wake-on-LAN Behaviour

The firmware always emits the WoL magic packet to the Ethernet broadcast address and repeats the broadcast MAC inside the payload, so no per-target configuration is required. Ensure the host you want to wake accepts broadcast WoL frames.

### Coding Style

Follow the project conventions documented in [`STYLE_GUIDE.md`](STYLE_GUIDE.md).

## Maintenance Commands

- `idf.py clean` – remove build artefacts while keeping configuration.
- `idf.py fullclean` – wipe the entire `build/` directory and CMake cache.
- `idf.py reconfigure` – regenerate build files after changing CMake or component deps.
- `idf.py size` – print a summary of flash and RAM usage.
- `idf.py erase-flash` – clear the entire flash chip before the next build.

## Tips

- Serial ports differ by OS: `/dev/ttyACM0` or `/dev/ttyUSB0` (Linux), `/dev/cu.usbserial-*` (macOS), `COMx` (Windows).
- To enable verbose build logs, append `-v` (for example `idf.py -v build`).
- Configuration defaults live in `sdkconfig.defaults`; rerun `idf.py menuconfig` if you edit them.
