# PocketBoy S3

PocketBoy S3 turns an M5Stack StickS3 into a private, offline Game Boy and Game Boy Color player for legally obtained homebrew games. The StickS3 hosts the WebUI and files; emulation runs locally in Chrome on a connected phone or tablet.

**Stable release: v1.0.0**

No native phone application, cloud service, account, telemetry, or internet connection is required.

## Features

- Plays `.gb` and `.gbc` homebrew ROMs through the bundled binjgb WebAssembly core.
- Supports DualSense, Xbox One, and touchscreen controls.
- Provides sound, pause, save states, battery-backed saves, and controller-only fullscreen.
- Stores homebrew games in a dedicated 2 MiB StickS3 library.
- Stores a compressed, checksummed save backup in a separate 1 MiB partition.
- Exports and imports all saves as one portable JSON file.
- Migrates saves between phones and tablets through either the JSON file or the StickS3.
- Displays battery level, charging state, Wi-Fi credentials, and a join QR code.

## What you need

- An M5Stack StickS3 with 8 MiB flash.
- A phone or tablet running the full Google Chrome browser.
- Optional DualSense or Xbox One Bluetooth controller.
- Legally obtained homebrew `.gb` or `.gbc` files. PocketBoy does not include ROMs.

## Install

1. Download the `PocketBoy-S3-v1.0.0.factory.bin` release asset.
2. Confirm the target is an M5Stack StickS3.
3. Write the factory image at flash offset `0x0000`.
4. Do not select **Erase entire flash** when updating if you want to preserve the S3 save backup and game library.
5. Restart the StickS3 and wait for the 2.5-second splash screen.

See [Flashing and recovery](docs/FLASHING.md) for detailed safety and data-preservation notes.

## Start playing

1. Join the `PocketBoy-XXXX` Wi-Fi network using the password displayed on the StickS3.
2. Open the full Chrome browser and visit `http://pocketboy`. If that name does not resolve, try `http://pocketboy.local`.
3. Open the game player.
4. Select a ROM from the phone or upload a homebrew ROM to the S3 library.
5. Tap **Sound On** once to satisfy Chrome's audio permission requirement.
6. Pair and activate a Bluetooth controller if desired.

## Controller mapping

| Controller input | Game Boy action |
| --- | --- |
| D-pad or left stick | D-pad |
| Circle / Xbox B | A |
| Cross / Xbox A | B |
| Options / Menu | Start |
| Create / View | Select |
| L2 | Enter or exit controller-only fullscreen |

PocketBoy applies the hardware-tested A/B and X/Y face-button corrections at its input layer. Touch controls remain available outside controller-only fullscreen.

## Saves and storage

| Location | Contents | Purpose |
| --- | --- | --- |
| Chrome | Per-ROM battery saves and save states | Active gameplay data |
| S3 save partition | One compressed backup, up to 420 KiB | Restore or move all Chrome saves |
| Downloaded JSON | Readable checksummed backup | Full-erase recovery and archival |
| S3 game partition | Homebrew ROM library, 2 MiB total | Launch without choosing the ROM again |

- **Save state / Load state** operate on the currently loaded ROM in Chrome.
- **Sync to S3** replaces the S3 backup after confirmation.
- **Restore S3** validates and merges that backup into Chrome, replacing only matching ROM saves.
- **Export saves** is the safest backup before a full-chip erase.
- Deleting a ROM from the game library does not delete its save data.
- Individual stored ROMs are limited to 1.8 MiB. Larger games can still use the phone file picker.

ROM identity is based on file contents, so the same ROM uses the same saves whether launched from the S3, phone, or tablet.

## Fullscreen and audio

- Chrome requires a user gesture before browser audio can begin; press **Sound On** after opening the player.
- Fullscreen is designed for a connected Bluetooth controller and hides the touchscreen controls.
- Press L2 or the WebUI fullscreen button to toggle it.
- PocketBoy requests landscape orientation and preserves the original 160×144 aspect ratio.

## Build from source

Install PlatformIO, clone this repository, and run:

```sh
platformio run -e m5stack-sticks3
```

The merged image is generated at:

```text
.pio/build/m5stack-sticks3/firmware.factory.bin
```

The custom 8 MiB partition map is defined in [`partitions.csv`](partitions.csv).

## Compatibility and validation

PocketBoy v1.0.0 has been field-tested with GB/GBC homebrew playback, audio, touchscreen input, DualSense and Xbox One controllers, save states, battery saves, fullscreen, S3 synchronization, phone-to-tablet save migration, and the on-device game library.

See [Release validation](docs/VALIDATION.md) for the regression checklist.

## Troubleshooting

- **No audio:** press **Sound On** or tap the page once. Chrome blocks autoplay until a user gesture.
- **Controller missing:** pair it in the device's Bluetooth settings, press a controller button, then use **Activate controller**.
- **Old interface after flashing:** close the old Chrome tab, reconnect to PocketBoy Wi-Fi, and reopen the player.
- **S3 storage unavailable:** create a manual JSON export before reflashing or troubleshooting the filesystem.
- **Save state rejected:** confirm that the ROM file is exactly the same build used to create the state.

## Privacy and legal use

PocketBoy operates locally on the StickS3 network. It does not upload games, saves, or usage information to the internet.

Use only games you created or have permission to use. No commercial ROMs, copyrighted game data, or console firmware are included.

## Third-party software

PocketBoy bundles the MIT-licensed [binjgb](https://github.com/binji/binjgb) WebAssembly emulator core at revision `16621111ed0ee73bcc45c912a823bcebedcffc0f`. See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) and `vendor/binjgb/` for license details.
