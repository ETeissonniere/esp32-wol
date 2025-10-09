# ESP32 WoL Code Style

This guide describes the conventions we follow across the ESP32 WoL firmware.
Treat it as the single source of truth when adding files or refactoring code.

## File Layout

- Group includes as: public header, ESP-IDF headers, project headers. Keep them
  alphabetised inside each group.
- Define module-wide state as `static` variables near the top of the file.
- Keep helper declarations `static` and `module_scope` (for example
  `wifi_app_*`, `ethernet_app_*`) to make ownership obvious.
- Prefer small, well-named helpers and let the exported function orchestrate
  the flow. See `main/wifi_app.c` and `main/ethernet_app.c` for the pattern.
- Indent with two spaces and place opening braces on the same line as the
  control statement.

## Error Handling

- All functions that can fail should return `esp_err_t`.
- Callers translate errors with `ESP_ERROR_CHECK` when failure is fatal.
- Inside helpers, return early on failure and propagate the `esp_err_t` value.
- When combining multiple steps, clean up or roll back partial state if a later
  call fails (for example unregistering an event handler if registration of the
  next handler fails).

## Logging

- Use the shared `TAG` macro from `main/common.h`.
- Log high-level transitions (`ESP_LOGI`) and only escalate to `ESP_LOGE` for
  unexpected or fatal errors.
- When logging values, prefer the IDF printf helpers (`IPSTR`, `MACSTR`, etc.)
  rather than manual formatting.

## Configuration & Constants

- Pull compile-time options from `sdkconfig` via `CONFIG_*` macros; avoid hard
  coded literals in source files.
- Use file-local `#define` statements (all caps) for bit masks or limits.

## Networking Patterns

- Wi-Fi and Ethernet bring-up should remain asynchronous. Register event
  handlers, expose a synchronous wait helper (using event groups), and do not
  block on driver APIs that expect callbacks.
- Create the relevant `esp_netif` instance in a dedicated helper and fail fast
  if allocation returns `NULL`.

## Comments & Naming

- Use comments sparingly. Add them only when the code is non-obvious or has
  ordering constraints.
- Name static helpers with a `module_action` pattern (`wifi_app_wait_for_connection`) so intent is clear at call sites.
- Keep identifiers ASCII-only unless interacting with external APIs that demand
  otherwise.

## Build & Maintenance

- Validate builds with `idf.py build` before committing.
- When introducing new dependencies, update `CMakeLists.txt` or `idf_component.yml` and verify `dependencies.lock` changes.
