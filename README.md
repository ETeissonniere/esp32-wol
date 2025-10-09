# ESP32 WOL (Ethernet)

Small ESP-IDF project that boots the Ethernet MAC/PHY and logs link and IP
information using the [`ethernet_init`](main/idf_component.yml) managed
component. The source lives in `main/esp32-wol.c`.

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
`idf.py set-target`.

## Quick Start Commands

Run the following from the repository root.

| Purpose | Command | Notes |
| --- | --- | --- |
| Configure project options (optional) | `idf.py menuconfig` | Only needed if you want to tweak defaults from `sdkconfig.defaults`. |
| Build the firmware | `idf.py build` | Outputs the application binary under `build/`. |
| Flash over USB | `idf.py -p /dev/ttyACM0 flash` | Replace the port with your OS-specific device path. |
| Flash and immediately monitor | `idf.py -p /dev/ttyACM0 flash monitor` | Press <kbd>Ctrl</kbd>+<kbd>]</kbd> to exit the monitor. |
| Monitor only | `idf.py -p /dev/ttyACM0 monitor` | Useful when the device is already flashed. |

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
