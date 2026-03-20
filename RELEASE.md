# Release Process

## Steps to Create a New Firmware Release

### 1. Update the Firmware Version

Edit `include/Config.h` and bump:

```cpp
constexpr char FW_VERSION[] = "0.4.0";
```

Use [Semantic Versioning](https://semver.org/):
- `MAJOR.MINOR.PATCH`
- `MAJOR`: breaking changes
- `MINOR`: new backward-compatible features
- `PATCH`: backward-compatible bug fixes

### 2. Build the Firmware

```bash
pio run
```

Expected output:

```text
.pio/build/esp32c3/firmware.bin
```

### 3. Check OTA Size Limit

The firmware must fit into one OTA app slot defined by `partitions_ota.csv`.

- Maximum firmware size: `1,835,008` bytes
- Release asset name must stay `firmware.bin`

If the binary exceeds the OTA slot size, GitHub OTA updates will fail.

### 4. Commit and Push

```bash
git add include/Config.h
git commit -m "Release v0.4.0"
git push
```

### 5. Create the GitHub Release

1. Open [GitHub Releases](https://github.com/TimonBed/Restarter/releases/new)
2. Create a tag matching the firmware version, for example `v0.4.0`
3. Use the same value as the release title, or add a short descriptive title
4. Add release notes
5. Upload `.pio/build/esp32c3/firmware.bin`
6. Rename the uploaded asset to `firmware.bin` if needed
7. Publish the release

### 6. Verify OTA Availability

After publishing:

- The device checks the latest GitHub release tag
- It compares that tag against `Config::FW_VERSION`
- It looks for a release asset named `firmware.bin`
- If a newer version exists, the web UI can offer the OTA update

## Quick Checklist

```text
[ ] Bump FW_VERSION in include/Config.h
[ ] Run pio run
[ ] Confirm .pio/build/esp32c3/firmware.bin exists
[ ] Confirm firmware size is below 1,835,008 bytes
[ ] Commit and push
[ ] Create GitHub release with matching tag
[ ] Upload firmware.bin asset
[ ] Test OTA update from the web UI
```
