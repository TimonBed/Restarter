#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

namespace OtaUpdateUtils {

int compareVersions(const String &a, const String &b);
void configureSecureClient(WiFiClientSecure &client, int timeoutMs);
void configureHttpsClient(HTTPClient &https, int timeoutMs);
bool httpsGetString(const String &url, String &responseBody, int timeoutMs, String &errorOut);

}  // namespace OtaUpdateUtils
