# Changelog

## v0.6.0-prototype

- Added a StickS3-hosted homebrew GB/GBC game library.
- Added WebUI controls to upload, refresh, launch, and delete stored games.
- Shows used and available game-library space.
- Sanitizes stored filenames, rejects duplicate names, and limits individual uploads to 1.8 MiB.
- Uses a temporary upload file so interrupted transfers never appear in the library.
- Keeps game storage separate from the 1 MiB save partition and preserves saves when a ROM is deleted.
- Flushes pending battery RAM before switching games and resets Pause for the new session.

## v0.5.0-prototype

- Added explicit Sync to S3 and Restore S3 controls to the player.
- Split the previous filesystem space into a dedicated 1 MiB save partition and a reserved 2 MiB game-library partition.
- Compresses the checksummed Chrome backup and stores it on the S3 with a safe 420 KiB payload limit.
- Uploads to a temporary file and protects the previous backup until replacement succeeds.
- Recovers an interrupted replacement on the next boot.
- Keeps manual JSON export/import available for full-chip erase recovery.

## v0.4.0-prototype

- Added one-file export for every PocketBoy battery save and save state in Chrome.
- Added checksum validation and ROM-aware import with duplicate and size checks.
- Validates the complete backup before writing and rolls back matching entries if storage fails.
- Merges imported entries without deleting saves for unrelated games.
- Closes the current game after import so it cannot overwrite newly restored battery data.

## v0.3.4-prototype

- Fixed an upstream binjgb `FileData` wrapper leak during state and cartridge-RAM operations.
- Frees both the temporary save payload and its outer WASM allocation.
- Rejects allocation failure instead of reading or writing an invalid save buffer.
- Keeps the 1.5-second startup lock and atomic state-loading safeguards.

## v0.3.3-prototype

- Locks Save State and Load State during the first 1.5 seconds after a ROM starts.
- Shows the locked state visually and announces when both controls are ready.
- Enforces the startup delay inside the player API as well as the WebUI.
- Keeps the atomic state-loading safeguards introduced in v0.3.2.

## v0.3.2-prototype

- Fixed intermittent corruption when loading a save state immediately after reloading a game.
- Loads state only after a browser-frame readiness barrier.
- Pauses emulation while applying the state and resets frame, audio, and rewind timelines before resuming.
- Validates save-state size and emulator acceptance before reporting success.
- Guarantees temporary WASM save buffers are released even when loading fails.

## v0.3.1-prototype

- Added explicit WebGL buffer, texture, shader, and program cleanup between games.
- Added explicit cleanup for completed and abandoned Web Audio sources.
- Fixed WASM ROM memory not being released after an invalid ROM load.
- Ends active rewind state cleanly before replacing a game.
- Prevented overlapping battery requests and reduced per-frame gamepad allocations.

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
