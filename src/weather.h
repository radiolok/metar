#ifndef __WEATHER_H__
#define __WEATHER_H__


#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

String get_metar_data(const String& icao_code);
void update_weather();

#endif