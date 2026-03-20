param(
  [string]$Port = ""
)

$ErrorActionPreference = "Stop"

function Get-UploadPort {
  param([string]$RequestedPort)

  if ($RequestedPort) {
    return $RequestedPort.Trim()
  }

  $json = & pio device list --json-output 2>$null
  if (-not $json) {
    throw "Could not detect a serial port automatically. Pass the port explicitly, e.g. upload_all.bat COM3"
  }

  $devices = $json | ConvertFrom-Json
  if ($devices -isnot [System.Array]) {
    $devices = @($devices)
  }

  $portDevices = @($devices | Where-Object { $_.port })
  if ($portDevices.Count -eq 0) {
    throw "No serial device detected. Pass the port explicitly, e.g. upload_all.bat COM3"
  }

  $espCandidates = @(
    $portDevices | Where-Object {
      $description = "$($_.description) $($_.hwid)"
      $description -match 'Espressif|303A:|USB JTAG|CDC|CP210|CH340'
    }
  )

  if ($espCandidates.Count -eq 1) {
    return $espCandidates[0].port
  }

  if ($portDevices.Count -eq 1) {
    return $portDevices[0].port
  }

  $ports = ($portDevices | ForEach-Object { $_.port } | Where-Object { $_ }) -join ", "
  throw "Multiple serial devices detected ($ports). Pass the target port explicitly, e.g. upload_all.bat COM3"
}

Get-UploadPort -RequestedPort $Port
