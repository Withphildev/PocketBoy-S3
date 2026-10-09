# Flashing and recovery

PocketBoy S3 targets the M5Stack StickS3 with 8 MiB flash. Confirm the board before writing firmware.

## Factory image

Use the merged `PocketBoy-S3-v1.0.0.factory.bin` release asset and write it at offset `0x0000`. The image contains the bootloader, partition table, boot metadata, and application.

A routine firmware write does not include the two data partitions:

| Partition | Address | Size | Contents |
| --- | --- | --- | --- |
| `saves` | `0x4F0000` | 1 MiB | Compressed Chrome save backup |
| `games` | `0x5F0000` | 2 MiB | Uploaded homebrew ROMs |

As long as the flashing tool writes the factory image without erasing the whole chip, these partitions remain intact.

## Before updating

1. Open the PocketBoy player.
2. Press **Sync to S3**.
3. Press **Export saves** and keep the downloaded JSON somewhere outside the device.
4. Close the old Chrome tab before reconnecting after the update.

The JSON export is essential before any full-chip erase because it lives outside both the browser's PocketBoy origin and the ESP32 flash.

## Normal update

1. Leave any **Erase entire flash**, **Erase all**, or equivalent option disabled.
2. Write the factory image at `0x0000`.
3. Restart the StickS3.
4. Reconnect to its Wi-Fi network and reopen `http://pocketboy` in Chrome.
5. Verify the S3 backup and game library are listed.

## Full-chip erase recovery

A full-chip erase removes the firmware, S3 save backup, and uploaded game library.

After reflashing:

1. Reconnect to PocketBoy Wi-Fi.
2. Open the player and press **Import saves**.
3. Select the previously exported JSON file.
4. Re-upload any desired homebrew ROMs.
5. Optionally press **Sync to S3** to recreate the on-device backup.

## Failure precautions

- Never disconnect power while a firmware image is being written.
- Game and save uploads use temporary files, so interrupted transfers should not appear as completed data.
- Keep a manual JSON export before partition-table experiments or recovery work.
- Do not flash this image onto a different ESP32-S3 board without reviewing its flash layout, display, power management, and pin assignments.
