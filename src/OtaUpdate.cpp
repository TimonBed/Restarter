#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>

#include "Config.h"
#include "OtaUpdate.h"

namespace {

// USERTrust RSA Certification Authority (used in GitHub TLS chain).
static const char kGithubRootCA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIF3jCCA8agAwIBAgIQAf1tMPyjylGoG7xkDjUDLTANBgkqhkiG9w0BAQwFADCB
iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl
cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV
BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAw
MjAxMDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNV
BAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVU
aGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBSU0EgQ2Vy
dGlmaWNhdGlvbiBBdXRob3JpdHkwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIK
AoICAQCAEmUXNg7D2wiz0KxXDXbtzSfTTK1Qg2HiqiBNCS1kCdzOiZ/MPans9s/B
3PHTsdZ7NygRK0faOca8Ohm0X6a9fZ2jY0K2dvKpOyuR+OJv0OwWIJAJPuLodMkY
tJHUYmTbf6MG8YgYapAiPLz+E/CHFHv25B+O1ORRxhFnRghRy4YUVD+8M/5+bJz/
Fp0YvVGONaanZshyZ9shZrHUm3gDwFA66Mzw3LyeTP6vBZY1H1dat//O+T23LLb2
VN3I5xI6Ta5MirdcmrS3ID3KfyI0rn47aGYBROcBTkZTmzNg95S+UzeQc0PzMsNT
79uq/nROacdrjGCT3sTHDN/hMq7MkztReJVni+49Vv4M0GkPGw/zJSZrM233bkf6
c0Plfg6lZrEpfDKEY1WJxA3Bk1QwGROs0303p+tdOmw1XNtB1xLaqUkL39iAigmT
Yo61Zs8liM2EuLE/pDkP2QKe6xJMlXzzawWpXhaDzLhn4ugTncxbgtNMs+1b/97l
c6wjOy0AvzVVdAlJ2ElYGn+SNuZRkg7zJn0cTRe8yexDJtC/QV9AqURE9JnnV4ee
UB9XVKg+/XRjL7FQZQnmWEIuQxpMtPAlR1n6BB6T1CZGSlCBst6+eLf8ZxXhyVeE
Hg9j1uliutZfVS7qXMYoCAQlObgOK6nyTJccBz8NUvXt7y+CDwIDAQABo0IwQDAd
BgNVHQ4EFgQUU3m/WqorSs9UgOHYm8Cd8rIDZsswDgYDVR0PAQH/BAQDAgEGMA8G
A1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEMBQADggIBAFzUfA3P9wF9QZllDHPF
Up/L+M+ZBn8b2kMVn54CVVeWFPFSPCeHlCjtHzoBN6J2/FNQwISbxmtOuowhT6KO
VWKR82kV2LyI48SqC/3vqOlLVSoGIG1VeCkZ7l8wXEskEVX/JJpuXior7gtNn3/3
ATiUFJVDBwn7YKnuHKsSjKCaXqeYalltiz8I+8jRRa8YFWSQEg9zKC7F4iRO/Fjs
8PRF/iKz6y+O0tlFYQXBl2+odnKPi4w2r78NBc5xjeambx9spnFixdjQg3IM8WcR
iQycE0xyNN+81XHfqnHd4blsjDwSXWXavVcStkNr/+XeTWYRUc+ZruwXtuhxkYze
Sf7dNXGiFSeUHM9h4ya7b6NnJSFd5t0dCy5oGzuCr+yDZ4XUmFF0sbmZgIn/f3gZ
XHlKYC6SQK5MNyosycdiyA5d9zZbyuAlJQG03RoHnHcAP9Dc1ew91Pq7P8yF1m9/
qS3fuQL39ZeatTXaw2ewh0qpKJ4jjv9cJ2vhsE/zB+4ALtRZh8tSQZXq9EfX7mRB
VXyNWQKV3WKdwrnuWih0hKWbt5DHDAff9Yk2dDLWKMGwsAvgnEzDHNb842m1R0aB
L6KCq9NjRHDEjf8tM7qtj3u1cIiuPhnPQCjY/MiQu12ZIvVS5ljFH4gxQ+6IHdfG
jjxDah2nGN59PRbxYvnKkKj9
-----END CERTIFICATE-----
)EOF";

struct OtaState {
  bool checking = false;
  bool updateInProgress = false;
  bool updateAvailable = false;
  bool lastCheckOk = false;
  bool rebootRequired = false;
  uint8_t progress = 0;
  uint32_t lastCheckMs = 0;
  String currentVersion = Config::FW_VERSION;
  String remoteVersion;
  String firmwareUrl;
  String filesystemUrl;
  String notes;
  String error;
};

struct OtaTaskParams {
  String firmwareUrl;
  String filesystemUrl;
};

SemaphoreHandle_t g_otaMutex = nullptr;
OtaState g_ota;
bool g_otaHasFilesystemStage = false;

constexpr char kLittleFsPartitionLabel[] = "littlefs";
constexpr size_t kFlashEraseSectorSize = 4096;

class OtaLock {
 public:
  OtaLock() : locked_(false) {
    if (g_otaMutex) {
      locked_ = (xSemaphoreTake(g_otaMutex, pdMS_TO_TICKS(2000)) == pdTRUE);
    }
  }

  ~OtaLock() {
    if (locked_) {
      xSemaphoreGive(g_otaMutex);
    }
  }

  bool locked() const { return locked_; }

 private:
  bool locked_;
};

int parseVersionPart(const String &token) {
  String t = token;
  t.trim();
  while (t.length() > 0 && !isDigit(t[0])) {
    t.remove(0, 1);
  }
  if (t.length() == 0) return 0;
  return t.toInt();
}

int compareVersions(const String &a, const String &b) {
  int ai[3] = {0, 0, 0};
  int bi[3] = {0, 0, 0};

  for (int i = 0, start = 0; i < 3 && start <= a.length(); i++) {
    int dot = a.indexOf('.', start);
    String part = (dot >= 0) ? a.substring(start, dot) : a.substring(start);
    ai[i] = parseVersionPart(part);
    if (dot < 0) break;
    start = dot + 1;
  }

  for (int i = 0, start = 0; i < 3 && start <= b.length(); i++) {
    int dot = b.indexOf('.', start);
    String part = (dot >= 0) ? b.substring(start, dot) : b.substring(start);
    bi[i] = parseVersionPart(part);
    if (dot < 0) break;
    start = dot + 1;
  }

  for (int i = 0; i < 3; i++) {
    if (ai[i] < bi[i]) return -1;
    if (ai[i] > bi[i]) return 1;
  }
  return 0;
}

bool httpsGetString(const String &url, String &responseBody, int timeoutMs, String &errorOut) {
  const int kMaxRetries = 3;
  const int kRetryDelayMs = 500;

  for (int attempt = 1; attempt <= kMaxRetries; attempt++) {
    WiFiClientSecure client;
#ifdef RESTARTER_DEV_AP_PASSWORD
    client.setInsecure();  // Skip TLS verify in dev (avoids cert chain issues)
#else
    client.setCACert(kGithubRootCA);
#endif
    client.setTimeout(timeoutMs / 1000);

    HTTPClient https;
    https.setTimeout(timeoutMs);
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!https.begin(client, url)) {
      errorOut = "HTTPS begin failed";
      return false;
    }

    https.addHeader("Accept", "application/vnd.github.v3+json");
    https.addHeader("User-Agent", "PC-Restarter-OTA/1.0");

    int code = https.GET();

    if (code == HTTP_CODE_OK) {
      responseBody = https.getString();
      https.end();
      return true;
    }

    https.end();

    if (code == -1) {
      errorOut = "Connection failed (WiFi/DNS/TLS)";
      if (attempt < kMaxRetries) {
        delay(kRetryDelayMs * attempt);
      }
    } else {
      errorOut = "HTTP " + String(code);
      return false;
    }
  }
  return false;
}

void updateProgress(size_t written, size_t total) {
  OtaLock lock;
  if (!lock.locked()) return;
  if (total > 0) {
    uint32_t pct = static_cast<uint32_t>((written * 100U) / total);
    if (pct > 100U) pct = 100U;
    if (g_otaHasFilesystemStage) {
      pct /= 2U;  // 0..50 for firmware, 50..100 for LittleFS
    }
    g_ota.progress = static_cast<uint8_t>(pct);
  }
}

void setProgressPercent(uint8_t pct) {
  OtaLock lock;
  if (!lock.locked()) return;
  g_ota.progress = pct;
}

void setTaskError(const String &error) {
  OtaLock lock;
  if (!lock.locked()) return;
  g_ota.error = error;
  g_ota.updateInProgress = false;
  g_ota.rebootRequired = false;
}

bool flashLittleFsImage(const String &url, String &errorOut) {
  const esp_partition_t *partition = esp_partition_find_first(
      ESP_PARTITION_TYPE_DATA,
      ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
      kLittleFsPartitionLabel);
  if (!partition) {
    errorOut = "LittleFS partition not found";
    return false;
  }

  const int kMaxRetries = 3;
  const int kRetryDelayMs = 500;

  for (int attempt = 1; attempt <= kMaxRetries; attempt++) {
    WiFiClientSecure client;
#ifdef RESTARTER_DEV_AP_PASSWORD
    client.setInsecure();
#else
    client.setCACert(kGithubRootCA);
#endif
    client.setTimeout(Config::OTA_DOWNLOAD_TIMEOUT_MS / 1000);

    HTTPClient https;
    https.setTimeout(Config::OTA_DOWNLOAD_TIMEOUT_MS);
    https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    if (!https.begin(client, url)) {
      errorOut = "Failed to initialize LittleFS request";
      return false;
    }

    int code = https.GET();
    if (code != HTTP_CODE_OK) {
      https.end();
      if (code == -1 && attempt < kMaxRetries) {
        delay(kRetryDelayMs * attempt);
        continue;
      }
      errorOut = "LittleFS download failed: HTTP " + String(code);
      return false;
    }

    int contentLength = https.getSize();
    if (contentLength <= 0) {
      https.end();
      errorOut = "LittleFS image has invalid size";
      return false;
    }

    size_t imageSize = static_cast<size_t>(contentLength);
    if (imageSize > partition->size) {
      https.end();
      errorOut = "LittleFS image too large";
      return false;
    }

    size_t eraseSize = (imageSize + kFlashEraseSectorSize - 1) & ~(kFlashEraseSectorSize - 1);
    esp_err_t eraseErr = esp_partition_erase_range(partition, 0, eraseSize);
    if (eraseErr != ESP_OK) {
      https.end();
      errorOut = "LittleFS erase failed";
      return false;
    }

    WiFiClient *stream = https.getStreamPtr();
    uint8_t buffer[1024];
    size_t written = 0;
    while (written < imageSize) {
      size_t remaining = imageSize - written;
      size_t toRead = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
      int bytes = stream->readBytes(reinterpret_cast<char *>(buffer), toRead);
      if (bytes <= 0) {
        https.end();
        errorOut = "LittleFS download interrupted";
        return false;
      }

      esp_err_t writeErr = esp_partition_write(partition, written, buffer, static_cast<size_t>(bytes));
      if (writeErr != ESP_OK) {
        https.end();
        errorOut = "LittleFS flash write failed";
        return false;
      }

      written += static_cast<size_t>(bytes);
      uint32_t stage = static_cast<uint32_t>((written * 50U) / imageSize);
      if (stage > 50U) stage = 50U;
      setProgressPercent(static_cast<uint8_t>(50U + stage));
    }

    https.end();
    return true;
  }

  errorOut = "LittleFS download failed";
  return false;
}

void otaTask(void *param) {
  OtaTaskParams *taskParams = static_cast<OtaTaskParams *>(param);
  String firmwareUrl = taskParams->firmwareUrl;
  String filesystemUrl = taskParams->filesystemUrl;
  delete taskParams;

  WiFiClientSecure client;
#ifdef RESTARTER_DEV_AP_PASSWORD
  client.setInsecure();
#else
  client.setCACert(kGithubRootCA);
#endif

  HTTPClient https;
  https.setTimeout(Config::OTA_DOWNLOAD_TIMEOUT_MS);
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!https.begin(client, firmwareUrl)) {
    setTaskError("Failed to initialize HTTPS update request");
    vTaskDelete(nullptr);
    return;
  }

  int code = https.GET();
  if (code != HTTP_CODE_OK) {
    https.end();
    setTaskError("Update download failed: HTTP " + String(code));
    vTaskDelete(nullptr);
    return;
  }

  int contentLength = https.getSize();
  if (contentLength <= 0) {
    https.end();
    setTaskError("Invalid update size");
    vTaskDelete(nullptr);
    return;
  }

  if (!Update.begin(static_cast<size_t>(contentLength), U_FLASH)) {
    https.end();
    setTaskError("Not enough OTA space");
    vTaskDelete(nullptr);
    return;
  }

  WiFiClient *stream = https.getStreamPtr();
  g_otaHasFilesystemStage = (filesystemUrl.length() > 0);
  Update.onProgress(updateProgress);
  size_t written = Update.writeStream(*stream);
  bool endOk = Update.end(true);
  g_otaHasFilesystemStage = false;
  https.end();

  if (written != static_cast<size_t>(contentLength) || !endOk || !Update.isFinished()) {
    String err = Update.errorString();
    if (err.length() == 0) {
      err = "OTA write failed";
    }
    setTaskError(err);
    vTaskDelete(nullptr);
    return;
  }

  if (filesystemUrl.length() > 0) {
    setProgressPercent(50);
    String fsError;
    if (!flashLittleFsImage(filesystemUrl, fsError)) {
      setTaskError(fsError);
      vTaskDelete(nullptr);
      return;
    }
  }

  {
    OtaLock lock;
    if (lock.locked()) {
      g_ota.progress = 100;
      g_ota.updateInProgress = false;
      g_ota.rebootRequired = true;
      g_ota.error = "";
    }
  }

  Serial.println("OTA update successful. Rebooting...");
  delay(1000);
  ESP.restart();
  vTaskDelete(nullptr);
}

}  // namespace

void OtaUpdate_setup() {
  if (!g_otaMutex) {
    g_otaMutex = xSemaphoreCreateMutex();
  }

  OtaLock lock;
  if (!lock.locked()) return;

  g_ota.currentVersion = Config::FW_VERSION;

  const esp_partition_t *running = esp_ota_get_running_partition();
  esp_ota_img_states_t otaState;
  if (esp_ota_get_state_partition(running, &otaState) == ESP_OK &&
      otaState == ESP_OTA_IMG_PENDING_VERIFY) {
    esp_err_t markOk = esp_ota_mark_app_valid_cancel_rollback();
    if (markOk == ESP_OK) {
      Serial.println("OTA: Current app marked valid.");
    } else {
      Serial.printf("OTA: Failed to mark app valid (err=%d)\n", static_cast<int>(markOk));
    }
  }
}

bool OtaUpdate_checkVersion() {
  {
    OtaLock lock;
    if (!lock.locked()) return false;
    if (g_ota.updateInProgress || g_ota.checking) {
      return false;
    }
    g_ota.checking = true;
    g_ota.lastCheckOk = false;
    g_ota.error = "";
  }

  String responseBody;
  String fetchError;
  bool ok = httpsGetString(Config::OTA_RELEASES_API, responseBody, Config::OTA_CHECK_TIMEOUT_MS, fetchError);
  String remoteVersion;
  String firmwareUrl;
  String filesystemUrl;
  String notes;

  if (ok) {
    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, responseBody);
    if (err) {
      ok = false;
      fetchError = "Invalid releases API response";
    } else {
      // tag_name e.g. "v0.3.0" -> strip leading "v"
      String tagName = doc["tag_name"] | "";
      if (tagName.length() > 0 && tagName[0] == 'v') {
        tagName.remove(0, 1);
      }
      remoteVersion = tagName;
      notes = doc["body"] | "";

      // Find firmware.bin in assets
      JsonArray assets = doc["assets"].as<JsonArray>();
      for (size_t i = 0; i < assets.size(); i++) {
        JsonObject asset = assets[i].as<JsonObject>();
        const char *name = asset["name"];
        if (name && strcmp(name, "firmware.bin") == 0) {
          firmwareUrl = asset["browser_download_url"] | "";
        } else if (name && strcmp(name, "littlefs.bin") == 0) {
          filesystemUrl = asset["browser_download_url"] | "";
        }
      }

      if (remoteVersion.length() == 0 || firmwareUrl.length() == 0) {
        ok = false;
        fetchError = "Release missing tag or firmware.bin";
      }
    }
  }

  OtaLock lock;
  if (!lock.locked()) return false;

  g_ota.checking = false;
  g_ota.lastCheckMs = millis();

  if (!ok) {
    g_ota.lastCheckOk = false;
    g_ota.updateAvailable = false;
    g_ota.remoteVersion = "";
    g_ota.firmwareUrl = "";
    g_ota.filesystemUrl = "";
    g_ota.notes = "";
    g_ota.error = fetchError;
    return false;
  }

  g_ota.lastCheckOk = true;
  g_ota.remoteVersion = remoteVersion;
  g_ota.firmwareUrl = firmwareUrl;
  g_ota.filesystemUrl = filesystemUrl;
  g_ota.notes = notes;
  g_ota.updateAvailable = (compareVersions(g_ota.currentVersion, remoteVersion) < 0);
  g_ota.error = "";
  return true;
}

bool OtaUpdate_startUpdate() {
  OtaLock lock;
  if (!lock.locked()) return false;

  if (g_ota.updateInProgress || g_ota.checking) return false;
  if (!WiFi.isConnected()) {
    g_ota.error = "WiFi not connected";
    return false;
  }
  if (!g_ota.updateAvailable || g_ota.firmwareUrl.length() == 0) {
    g_ota.error = "No update available";
    return false;
  }

  g_ota.updateInProgress = true;
  g_ota.rebootRequired = false;
  g_ota.progress = 0;
  g_ota.error = "";

  OtaTaskParams *taskParams = new OtaTaskParams{g_ota.firmwareUrl, g_ota.filesystemUrl};
  BaseType_t taskOk = xTaskCreate(
      otaTask,
      "ota_task",
      12288,
      taskParams,
      1,
      nullptr);

  if (taskOk != pdPASS) {
    delete taskParams;
    g_ota.updateInProgress = false;
    g_ota.error = "Failed to start OTA task";
    return false;
  }
  return true;
}

bool OtaUpdate_isBusy() {
  OtaLock lock;
  if (!lock.locked()) return false;
  return g_ota.checking || g_ota.updateInProgress;
}

String OtaUpdate_getStatusJson() {
  OtaLock lock;
  StaticJsonDocument<512> doc;
  if (lock.locked()) {
    doc["checking"] = g_ota.checking;
    doc["updateInProgress"] = g_ota.updateInProgress;
    doc["available"] = g_ota.updateAvailable;
    doc["lastCheckOk"] = g_ota.lastCheckOk;
    doc["rebootRequired"] = g_ota.rebootRequired;
    doc["progress"] = g_ota.progress;
    doc["lastCheckMs"] = g_ota.lastCheckMs;
    doc["currentVersion"] = g_ota.currentVersion;
    doc["remoteVersion"] = g_ota.remoteVersion;
    doc["notes"] = g_ota.notes;
    doc["error"] = g_ota.error;
  } else {
    doc["error"] = "OTA status unavailable";
  }

  String out;
  serializeJson(doc, out);
  return out;
}
