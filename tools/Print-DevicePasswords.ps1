param(
  [string]$Port = ""
)

$ErrorActionPreference = "Stop"

function Get-RepoFilePath {
  param([string]$RelativePath)
  return Join-Path (Split-Path -Parent $PSScriptRoot) $RelativePath
}

function Test-DevPasswordMode {
  $platformioPath = Get-RepoFilePath "platformio.ini"
  foreach ($line in Get-Content $platformioPath) {
    $trimmed = $line.Trim()
    if (-not $trimmed -or $trimmed.StartsWith(";")) {
      continue
    }
    if ($trimmed -match "(^|\s)-DRESTARTER_DEV_AP_PASSWORD(\s|$)") {
      return $true
    }
  }
  return $false
}

function Get-DevPasswordValue {
  param(
    [string]$ConstantName,
    [string]$FallbackValue
  )

  $configPath = Get-RepoFilePath "include\Config.h"
  $content = Get-Content $configPath -Raw
  $pattern = [regex]::Escape($ConstantName) + '\[\]\s*=\s*"([^"]+)"'
  $match = [regex]::Match($content, $pattern)
  if ($match.Success) {
    return $match.Groups[1].Value
  }
  return $FallbackValue
}

function Get-UploadPort {
  param([string]$RequestedPort)

  if ($RequestedPort) {
    return $RequestedPort
  }

  $json = & pio device list --json-output 2>$null
  if (-not $json) {
    throw "Could not detect a serial port automatically. Pass the port explicitly, e.g. upload_all.bat COM3"
  }

  $devices = $json | ConvertFrom-Json
  if ($devices -isnot [System.Array]) {
    $devices = @($devices)
  }

  if ($devices.Count -eq 1 -and $devices[0].port) {
    return $devices[0].port
  }

  if ($devices.Count -eq 0) {
    throw "No serial device detected. Pass the port explicitly, e.g. upload_all.bat COM3"
  }

  $ports = ($devices | ForEach-Object { $_.port } | Where-Object { $_ }) -join ", "
  throw "Multiple serial devices detected ($ports). Pass the target port explicitly, e.g. upload_all.bat COM3"
}

function Get-MacFromEsptool {
  param([string]$ResolvedPort)

  for ($attempt = 1; $attempt -le 10; $attempt++) {
    $output = & pio pkg exec -p tool-esptoolpy -- esptool.py --chip esp32c3 --port $ResolvedPort read_mac 2>&1 | Out-String
    $match = [regex]::Match($output, 'MAC:\s*([0-9A-Fa-f:]{17})')
    if ($match.Success) {
      return $match.Groups[1].Value.ToLowerInvariant()
    }
    Start-Sleep -Milliseconds 700
  }

  throw "Unable to read the device MAC from $ResolvedPort after flashing."
}

function Get-GeneratedPassword {
  param(
    [UInt64]$Mac,
    [UInt32]$Salt
  )

  $chars = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ"
  [UInt32]$hash = $Salt

  for ($i = 0; $i -lt 6; $i++) {
    [UInt32]$byte = ($Mac -shr ($i * 8)) -band 0xFF
    $hash = [UInt32]((((([UInt64]$hash) -shl 5) + $hash) -bxor $byte))
  }

  $hash = [UInt32]($hash -bxor ($hash -shr 16))

  $passwordChars = New-Object char[] 8
  for ($i = 0; $i -lt 8; $i++) {
    $passwordChars[$i] = $chars[[int]($hash % 32)]
    $hash = [UInt32]($hash / 32)
    $hash = [UInt32]($hash -bxor ($Mac -shr ($i * 5)))
  }

  return -join $passwordChars
}

if (Test-DevPasswordMode) {
  $apPassword = Get-DevPasswordValue -ConstantName "AP_PASSWORD_DEV" -FallbackValue "Test1234"
  $adminPassword = Get-DevPasswordValue -ConstantName "ADMIN_PASSWORD_DEV" -FallbackValue "Klaus1234"

  Write-Host ""
  Write-Host "Restarter credentials (dev mode):"
  Write-Host "WiFi password : $apPassword"
  Write-Host "Admin user    : admin"
  Write-Host "Admin password: $adminPassword"
  exit 0
}

$resolvedPort = Get-UploadPort -RequestedPort $Port
$macAddress = Get-MacFromEsptool -ResolvedPort $resolvedPort
$deviceId = $macAddress.Replace(":", "")
$macValue = [Convert]::ToUInt64($deviceId, 16)

$apPassword = Get-GeneratedPassword -Mac $macValue -Salt 0x5A3C9E7B
$adminPassword = Get-GeneratedPassword -Mac $macValue -Salt 0x7B9E3C5A
$ssid = "Restarter-" + $deviceId.Substring(6)
$hostname = "restarter-" + $deviceId

Write-Host ""
Write-Host "Restarter credentials:"
Write-Host "Port          : $resolvedPort"
Write-Host "SSID          : $ssid"
Write-Host "WiFi password : $apPassword"
Write-Host "Admin user    : admin"
Write-Host "Admin password: $adminPassword"
Write-Host "Hostname      : $hostname.local"
