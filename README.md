# PocketBoy S3

PocketBoy S3 is an offline, browser-based Game Boy and Game Boy Color player hosted by an M5Stack StickS3.

> **Playable prototype — v0.4.0.** This build adds local GB/GBC emulation in the phone browser. It still needs broad game, audio, save, and mobile-device testing.

## Current prototype

- Creates a unique `PocketBoy-XXXX` Wi-Fi network and device-specific password.
- Hosts an offline WebUI at `http://pocketboy` with `pocketboy.local` as an mDNS fallback.
- Redirects captive-portal checks to the WebUI.
- Tests Chrome visibility and mappings for DualSense and Xbox controllers.
- Applies the hardware-tested face-button corrections: A/B and X/Y are swapped at the PocketBoy input layer.
- Runs user-selected `.gb` and `.gbc` ROMs locally in the browser using the binjgb WebAssembly core.
- Provides browser audio, fullscreen play, touchscreen controls, pause, save states, and per-ROM battery-save storage.
- Merges touchscreen and physical-controller input without either source cancelling the other.
- Provides explicit Sound On/Off and Enter/Exit Fullscreen controls.
- Uses the complete phone display in fullscreen while preserving the original game aspect ratio.
- Keeps fullscreen as a clean Bluetooth-controller-only mode with no touchscreen overlay.
- Requires a detected controller before entering fullscreen and exits if it disconnects.
- Maps L2 to toggle fullscreen and requests landscape orientation while fullscreen is active.
- Falls back to a browser-independent fixed screen mode when Chrome blocks the Fullscreen API.
- Rotates the game surface with CSS when Chrome cannot lock the device orientation.
- Prevents Chrome from reusing stale player controls after a firmware update.
- Keeps the fullscreen container on the physical viewport axes and rotates only the canvas in portrait mode.
- Shows the gray-handheld PocketBoy artwork for 2.5 seconds while the Wi-Fi portal starts.
- Shows live battery percentage and charging state on the StickS3 and in both WebUI pages.
- Explicitly releases browser GPU, audio, rewind, and WASM resources when games are changed.
- Loads save states atomically after resetting frame, audio, and rewind timelines.
- Locks Save State and Load State for the first 1.5 seconds after each game starts.
- Fully releases temporary WASM save buffers used by save states and automatic cartridge saves.
- Exports every browser save as one checksummed PocketBoy backup file.
- Validates and imports backups without deleting saves for unrelated games.
- Shows live buttons, D-pad, left stick, Start, and Select input.
- Displays Wi-Fi onboarding or a join QR code on the StickS3.
- Dims the display after 30 seconds while keeping the portal active.
- Uses no cloud service, account, telemetry, or native phone application.

## Build

```sh
platformio run
```

Flash the generated factory image at offset `0x0000` only after confirming the target is an M5Stack StickS3.

## Next milestone

Validate Save Manager export/import on Chrome, then add an optional StickS3-hosted game library and save synchronization.

## Third-party software

PocketBoy bundles the MIT-licensed [binjgb](https://github.com/binji/binjgb) WebAssembly emulator core at revision `16621111ed0ee73bcc45c912a823bcebedcffc0f`. See `THIRD_PARTY_NOTICES.md` and `vendor/binjgb/` for license details.
