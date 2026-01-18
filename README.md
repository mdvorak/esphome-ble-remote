# BLE Remote Component

This ESPHome custom component provides both **BLE Remote Transmitter** and **BLE Remote Receiver** functionality in a single unified component.

## Features

- **Transmitter Mode**: Send BLE advertisements using ESP32 BLE Server
- **Receiver Mode**: Listen for BLE advertisements using ESP32 BLE Tracker
- **Unified Component**: Both modes use the same namespace and component name
- **Separate Headers**: Maintains separate `ble_remote.h` and `ble_remote_receiver.h` files

## Configuration

### Transmitter Mode (BLE Remote)

Use this to send BLE remote signals:

```yaml
ble_remote:
  - id: my_ble_remote
    ble_server_id: ble_server
    remote_id: 0x1234
```

**Required parameters:**
- `ble_server_id`: Reference to an existing `esp32_ble_server` component
- `remote_id`: Unique 16-bit hex identifier for this remote

**Actions:**
```yaml
- ble_remote.toggle:
    id: my_ble_remote
```

### Receiver Mode (BLE Remote Receiver)

Use this to receive BLE remote signals:

```yaml
ble_remote:
  - id: my_ble_receiver
    mac_address: AA:BB:CC:DD:EE:FF
    remote_id: 0x1234
    on_toggle:
      - logger.log: "Remote toggled!"
```

**Required parameters:**
- `mac_address`: MAC address of the transmitter device to listen for
- `remote_id`: The 16-bit hex identifier to match (must match transmitter's `remote_id`)

**Optional parameters:**
- `on_toggle`: Automation trigger that fires when the remote value changes

## Example Usage

### Complete Example: Transmitter Device (Remote Control)

```yaml
esphome:
  name: living-room-switch

esp32:
  board: esp32dev

esp32_ble_server:
  id: ble_server

ble_remote:
  id: light_remote
  ble_server_id: ble_server
  remote_id: 0xEFFF

button:
  - platform: gpio
    pin: GPIO0
    on_press:
      - ble_remote.toggle:
          id: light_remote
```

### Complete Example: Receiver Device (Light Controller)

```yaml
esphome:
  name: living-room-light

esp32:
  board: esp32dev

esp32_ble_tracker:
  scan_parameters:
    active: false

ble_remote_receiver:
  id: ble_remote_rx
  mac_address: "24:0A:C4:XX:XX:XX"  # MAC of transmitter device
  remote_id: 0xEFFF
  on_toggle:
    - lambda: |-
        ESP_LOGI("ble_remote_rx", "Switch toggle detected: %u", x);
    - light.toggle: main_light

light:
  - platform: binary
    id: main_light
    output: light_output
```

## How It Works

The **ble_remote** component (transmitter) sends a random 32-bit value via BLE manufacturer data each time its `toggle()` action is called. The **ble_remote_receiver** component (receiver) listens for BLE advertisements from a specific MAC address and triggers automations when the transmitted value changes.

## Technical Details

### Transmitter (ble_remote)
- **Namespace**: `ble_remote`
- **Class**: `BLERemote`
- **Dependency**: `esp32_ble_server`
- **Protocol**: Uses BLE manufacturer data with `remote_id` as manufacturer UUID

### Receiver (ble_remote_receiver)
- **Namespace**: `ble_remote` (shared namespace)
- **Class**: `BLERemoteReceiver`
- **Dependency**: `esp32_ble_tracker`
- **Trigger**: Provides `on_toggle` automation trigger with uint32 parameter
