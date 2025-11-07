#ifndef __WEATHER_H__
#define __WEATHER_H__


#include <Arduino.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

#include "Metar.h"
using namespace Storage_B::Weather;

String get_metar_data(const String& icao_code);
std::shared_ptr<Metar> update_weather(const String& icao_code);

#endif