#pragma once

#include <Arduino.h>

void OtaUpdate_setup();
bool OtaUpdate_checkVersion();
bool OtaUpdate_startUpdate();
bool OtaUpdate_isBusy();
String OtaUpdate_getStatusJson();
