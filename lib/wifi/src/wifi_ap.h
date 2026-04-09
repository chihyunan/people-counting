#ifndef WIFI_AP_H
#define WIFI_AP_H

#include <Arduino.h>

// SoftAP + HTTP :80. Reads *inRoom, *entered, *exited (owned by sketch).
void wifiBegin(const char *apSsid, const char *apPass, int *inRoom, unsigned long *entered,
               unsigned long *exited);

void wifiLoop();

#endif
