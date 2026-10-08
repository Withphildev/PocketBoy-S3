# Changelog

## v0.2.2-prototype

- Added an explicit Sound On/Off control that satisfies Chrome's audio activation requirement.
- Added a reliable Enter/Exit Fullscreen toggle with a dedicated fullscreen player layout.
- Kept volume selection independent so the chosen level is restored when sound is enabled.

## v0.2.1-prototype

- Fixed connected gamepads repeatedly cancelling touchscreen D-pad and button input.
- Added touch-cancel handling to prevent controls from becoming stuck.
- Added live visual feedback on the touchscreen controls for physical controller input.

## v0.2.0-prototype

- Integrated the MIT-licensed binjgb WebAssembly GB/GBC emulator core.
- Added phone-local `.gb` and `.gbc` file selection.
- Added canvas video, Web Audio output, fullscreen play, and touchscreen controls.
- Added the hardware-tested DualSense/Xbox controller mapping to the emulator.
- Added per-ROM battery-save storage and browser-local save states.
- Preserved the controller diagnostic page as a separate compatibility tool.

## v0.1.1-prototype

- Swapped the A and B face-button inputs following physical controller testing.
- Swapped the X and Y face-button inputs following physical controller testing.
- Updated the WebUI mapping guide to show the corrected PocketBoy controls.

## v0.1.0-prototype

- Added the initial M5StickS3 firmware foundation.
- Added private AP mode, captive DNS, mDNS, and QR onboarding.
- Added a responsive Chrome controller compatibility lab.
- Added live standard-mapping visualization for DualSense and Xbox controllers.
- Added a dimmed low-refresh StickS3 status display.
