# Changelog

## v0.3.0-prototype

- Added a color-coded battery gauge to the StickS3 status screen.
- Added live battery percentage, voltage, and charging state to the controller lab.
- Added a compact live battery indicator to the game player.
- Refreshes battery information every five seconds without interrupting gameplay.

## v0.2.9-prototype

- Replaced the temporary drawn boot screen with the gray-handheld PocketBoy artwork.
- Optimized the artwork for the StickS3's native 240×135 landscape display.
- Matched the other Pocket app's 2.5-second splash duration.

## v0.2.8-prototype

- Added a PocketBoy S3 boot splash to the StickS3 display.
- Shows PocketBoy branding, GB/GBC support, firmware version, and startup progress.
- Keeps the splash visible for at least 1.5 seconds before showing Wi-Fi connection details.

## v0.2.7-prototype

- Fixed portrait fullscreen clipping caused by rotating the fullscreen container itself.
- Kept the fullscreen container aligned to the device viewport and rotates only the game canvas.
- Added dynamic viewport units so Chrome's browser controls do not distort the fullscreen dimensions.

## v0.2.6-prototype

- Removed the controller-detection lock from the fullscreen button while keeping fullscreen controller-only.
- Fixed L2 detection for controllers that report an analog trigger value without setting `pressed`.
- Disabled browser caching for the player page and JavaScript so firmware updates take effect immediately.
- Added visible Pause, Save State, and Load State success and error feedback.

## v0.2.5-prototype

- Replaced the hard dependency on Chrome's Fullscreen API with an immediate CSS screen-mode fallback.
- Made L2 and the WebUI button toggle the same controller-only screen mode.
- Added a 90-degree CSS rotation fallback when Chrome cannot lock landscape orientation.
- Ensured the game remains aspect-correct and covers the available viewport with all controls hidden.

## v0.2.4-prototype

- Mapped the L2 trigger to toggle fullscreen on its press edge.
- Changed fullscreen to contain only the game screen, with no surrounding player UI.
- Requests landscape orientation after fullscreen begins and unlocks orientation on exit.
- Added clear feedback when Chrome rejects controller-initiated fullscreen because it requires a tap.

## v0.2.3-prototype

- Changed fullscreen playback to use the complete phone or tablet display.
- Preserved the 160×144 game aspect ratio without stretching or cropping.
- Made fullscreen a Bluetooth-controller-only mode with no touchscreen overlay.
- Requires a detected controller before entering fullscreen.
- Automatically exits fullscreen if the Bluetooth controller disconnects.

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
