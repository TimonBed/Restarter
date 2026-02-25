#include "OtaUpdateUtils.h"

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

constexpr int kHttpMaxRetries = 3;
constexpr int kHttpRetryDelayMs = 500;

int parseVersionPart(const String &token) {
  String t = token;
  t.trim();
  while (t.length() > 0 && !isDigit(t[0])) {
    t.remove(0, 1);
  }
  if (t.length() == 0) return 0;
  return t.toInt();
}

void parseVersion(const String &version, int out[3]) {
  out[0] = 0;
  out[1] = 0;
  out[2] = 0;

  for (int i = 0, start = 0; i < 3 && start <= version.length(); i++) {
    int dot = version.indexOf('.', start);
    String part = (dot >= 0) ? version.substring(start, dot) : version.substring(start);
    out[i] = parseVersionPart(part);
    if (dot < 0) break;
    start = dot + 1;
  }
}

}  // namespace

namespace OtaUpdateUtils {

int compareVersions(const String &a, const String &b) {
  int ai[3] = {0, 0, 0};
  int bi[3] = {0, 0, 0};

  parseVersion(a, ai);
  parseVersion(b, bi);

  for (int i = 0; i < 3; i++) {
    if (ai[i] < bi[i]) return -1;
    if (ai[i] > bi[i]) return 1;
  }
  return 0;
}

void configureSecureClient(WiFiClientSecure &client, int timeoutMs) {
#ifdef RESTARTER_DEV_AP_PASSWORD
  client.setInsecure();  // Skip TLS verify in dev (avoids cert chain issues)
#else
  client.setCACert(kGithubRootCA);
#endif
  client.setTimeout(timeoutMs / 1000);
}

void configureHttpsClient(HTTPClient &https, int timeoutMs) {
  https.setTimeout(timeoutMs);
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
}

bool httpsGetString(const String &url, String &responseBody, int timeoutMs, String &errorOut) {
  for (int attempt = 1; attempt <= kHttpMaxRetries; attempt++) {
    WiFiClientSecure client;
    configureSecureClient(client, timeoutMs);

    HTTPClient https;
    configureHttpsClient(https, timeoutMs);
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
      if (attempt < kHttpMaxRetries) {
        delay(kHttpRetryDelayMs * attempt);
      }
    } else {
      errorOut = "HTTP " + String(code);
      return false;
    }
  }
  return false;
}

}  // namespace OtaUpdateUtils
