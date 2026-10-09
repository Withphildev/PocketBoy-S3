# Release validation

PocketBoy S3 v1.0.0 is based on the field-tested v0.6.0 feature set. Run this checklist when changing the emulator, WebUI, partition map, or storage code.

## Firmware and portal

- Splash remains visible for approximately 2.5 seconds.
- StickS3 displays the unique Wi-Fi name and password.
- QR screen and display dimming work.
- `http://pocketboy` opens in the full Chrome browser.
- Battery percentage and charging state update without interrupting gameplay.

## Game playback

- A local `.gb` ROM starts from the phone or tablet picker.
- A local `.gbc` ROM starts from the picker.
- A stored game uploads, appears in the library, and launches.
- Duplicate stored filenames are rejected without replacing the original.
- Deleted ROMs disappear while their saves remain available after re-upload.
- Switching games preserves pending battery RAM and resets Pause state.

## Input, audio, and display

- Touch D-pad, A, B, Start, and Select work.
- DualSense and Xbox One controllers are detected.
- Corrected A/B and X/Y mappings match the controller lab.
- Sound begins after a user gesture and can be muted and restored.
- L2 enters and exits controller-only fullscreen.
- Portrait devices rotate the game surface and preserve the 160×144 aspect ratio.

## Saves

- Save State and Load State remain locked for the first 1.5 seconds after launch.
- Repeated save/load cycles work without refreshing the page.
- Battery-backed progress survives a ROM reload.
- JSON export contains all PocketBoy save records.
- JSON import restores matching saves without deleting unrelated saves.
- Corrupted backup checksums are rejected.
- Sync to S3 and Restore S3 complete successfully.
- Saves migrate between a phone and tablet.

## Persistence and recovery

- A normal factory-image reflash preserves the S3 save backup.
- A normal factory-image reflash preserves the S3 game library.
- A manual JSON export restores saves after a full-chip erase.
- Storage-full and oversized-file errors leave existing data usable.

## Automated checks

- `node --check vendor/binjgb/player.js`
- Every inline WebUI script compiles with `new Function(...)`.
- `git diff --check`
- `platformio run -e m5stack-sticks3`
- The factory binary contains the expected v1.0.0 WebUI and firmware markers.
