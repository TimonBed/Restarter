# Release Process

## Steps to Create a New Firmware Release

### 1. Update Version Number

Edit `include/Config.h` and update the version:

```cpp
constexpr char FW_VERSION[] = "0.3.0";  // Change to new version
```

### 2. Build the Firmware

```bash
pio run
```

The compiled firmware will be at:
```
.pio/build/esp32c3/firmware.bin
```

**Firmware Size Limit:** Maximum **1,835,008 bytes (1.79 MB / 1792 KB)**

Each OTA slot (`app0` and `app1`) is 1.79 MB. The firmware binary must fit within this limit. Current firmware is ~1.17 MB, leaving ~622 KB headroom.

If the binary exceeds 1.79 MB:
- The OTA update will fail
- Consider optimizing code or reducing features
- Or adjust partition sizes in `partitions_ota.csv` (reduces LittleFS space)

### 3. Commit and Push Changes

```bash
git add include/Config.h
git commit -m "Release v0.3.0"
git push
```

### 4. Create GitHub Release

1. Go to: https://github.com/TimonBed/Restarter/releases/new
2. **Tag version:** `v0.3.0` (must match the version in Config.h)
3. **Release title:** `v0.3.0` or descriptive name
4. **Description:** Release notes (what changed)
5. **Attach binary:** Upload `.pio/build/esp32c3/firmware.bin` as `firmware.bin`
6. Click **"Publish release"**

### 5. Verify OTA Update

After publishing:
- Devices can check for updates via the web UI (Settings → Firmware Update)
- The OTA system fetches the latest GitHub release tag and compares versions
- Looks for `firmware.bin` in release assets
- If a newer version is available, users can download and install it

## Version Numbering

Use [Semantic Versioning](https://semver.org/):
- **MAJOR.MINOR.PATCH** (e.g., `0.3.0`)
- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes

## Quick Release Script

```bash
# Set version (edit manually in Config.h first)
VERSION="0.3.0"

# Build
pio run

# Copy firmware
cp .pio/build/esp32c3/firmware.bin firmware.bin

echo "Next steps:"
echo "1. Commit: git add include/Config.h && git commit -m 'Release v$VERSION'"
echo "2. Push: git push"
echo "3. Create GitHub release: https://github.com/TimonBed/Restarter/releases/new"
echo "4. Upload firmware.bin as asset (must be named firmware.bin)"
```
