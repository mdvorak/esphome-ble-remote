# BLE Remote Components

Custom ESPHome components for sending and receiving short authenticated commands over BLE
manufacturer data.

The set is split into three components:

- `ble_remote_common` — shared support code (HMAC, command struct). Auto-loaded; not declared
  directly in YAML.
- `ble_remote` — transmitter. Sends a `(command, nonce, hash)` payload via the ESP32 BLE
  advertisement.
- `ble_remote_receiver` — receiver. Listens for advertisements from a known MAC, validates the
  HMAC against a shared key, and fires automation triggers.

## Wire format

Each transmission goes out as BLE manufacturer data (AD type 0xFF):

| Offset | Size  | Field      | Notes                                                                |
|--------|-------|------------|----------------------------------------------------------------------|
| 0      | 2 B   | company ID | Fixed `0xFFFF` — the Bluetooth SIG's reserved testing/dev identifier |
| 2      | 2 B   | command    | User-defined 16-bit command code                                     |
| 4      | 4 B   | nonce      | Random per-transmission, never zero, never equal to previous nonce   |
| 8      | 8 B   | hash       | HMAC-SHA256(key, command \|\| nonce) truncated to 8 bytes            |

Total: 16 bytes of manufacturer data. The receiver filters incoming frames to only
those carrying company ID `0xFFFF` from the configured MAC.

The receiver maintains a 16-entry replay window of recently-seen valid nonces. To avoid
replaying a packet captured before reboot, the transmitter broadcasts a `nonce=0` boot
sentinel from `setup()` until the first real press; the receiver treats `nonce=0` as
"ready, do not trigger". As a fallback, if the receiver boots while the transmitter is
already broadcasting a real (non-zero) command, the first valid command after boot is
absorbed once instead of being replayed.

## Transmitter — `ble_remote`

```yaml
esp32_ble_server:

ble_remote:
  id: light_remote
  shared_key: !secret ble_remote_key   # min 8 chars
```

**Options**

| Option         | Type            | Required | Description                                                      |
|----------------|-----------------|----------|------------------------------------------------------------------|
| `id`           | id              | yes      | Component id                                                     |
| `ble_server_id`| id              | no       | `esp32_ble_server` instance (auto-resolved if only one is defined) |
| `shared_key`   | string (≥ 8)    | yes      | HMAC key shared with the receiver                                |

**Action: `ble_remote.write`**

```yaml
button:
  - platform: gpio
    pin: GPIO0
    on_press:
      - ble_remote.write:
          id: light_remote
          command: 0x0001
```

| Option    | Type             | Required | Description                |
|-----------|------------------|----------|----------------------------|
| `id`      | id               | yes      | Target `ble_remote`        |
| `command` | hex uint16       | yes      | 16-bit command code        |

## Receiver — `ble_remote_receiver`

```yaml
esp32_ble_tracker:
  scan_parameters:
    active: false

ble_remote_receiver:
  id: light_rx
  setup_priority: -900                 # set up after other components if needed
  mac_address: "24:0A:C4:XX:XX:XX"     # MAC of the transmitter
  shared_key: !secret ble_remote_key   # must match transmitter
  on_command:
    - command: 0x0001
      then:
        - light.toggle: main_light
    - then:
        - lambda: |-
            ESP_LOGI("ble_remote", "received command 0x%04X", x);
```

**Options**

| Option        | Type            | Required | Description                                |
|---------------|-----------------|----------|--------------------------------------------|
| `id`          | id              | yes      | Component id                               |
| `mac_address` | mac             | yes      | MAC of the transmitter to listen for       |
| `shared_key`  | string (≥ 8)    | yes      | Must match the transmitter's shared key    |
| `on_command`  | automation list | no       | Triggers fired on validated commands       |

**Trigger: `on_command`**

Each entry under `on_command` is an automation. Optional `command:` filter limits the trigger
to a specific 16-bit code; omit it to receive all validated commands. The trigger exposes the
received command as `x` (`uint32_t`).

| Option    | Type        | Required | Description                                            |
|-----------|-------------|----------|--------------------------------------------------------|
| `command` | hex uint16  | no       | If set, fire only when the received command matches    |

## Security notes

This component is sized for normal home switches: lights, fans, scenes. It's intended to keep
random nearby BLE chatter from triggering your switches, not to withstand a focused attacker.

- Each packet is authenticated by HMAC-SHA256 over `(command, nonce)` keyed with the shared
  key, so anyone without the key can't fabricate or alter commands by sniffing the air.
- A 16-entry replay window plus a `nonce=0` boot sentinel (and a fallback skip of the first
  valid packet after boot) handles incidental re-broadcasts and short-term cross-reboot replays.
- BLE itself can't be made fully tamper-proof on commodity hardware — the keys live in flash,
  advertisements are public, and a determined attacker on-site can always do more. If you're
  building anything safety-critical (locks, garage doors, alarms), use a different transport
  (Wi-Fi+TLS, Matter, Zigbee with proper pairing); don't try to harden this component up to
  that bar.

## Layout

```
ble_components/
  ble_remote_common/   # shared header, HMAC implementation
  ble_remote/          # transmitter component + ble_remote.write action
  ble_remote_receiver/ # receiver component + on_command trigger
```
