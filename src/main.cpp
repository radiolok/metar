#include <Arduino.h>
#include "main.h"
#if defined(ESP32)
#include <SPIFFS.h>
#elif defined(ESP8266)
#include <FS.h>
#endif
#include "eertos.h"
#include <Ticker.h>
#include <ESPAsyncWebServer.h>
#include "FSWebServerLib.h"
#include "terminal.h"

#include "krams.h"
#include "weather.h"

Ticker _secondEERtos;
void usecondTick();
void secondTick();
void tenSecondTick();

void setup() {
    Serial.begin(115200);
    delay(1000);
    InitRTOS(); // init eertos
    _secondEERtos.attach_ms(1, &usecondTick); // init eertos time manager
    _secondEERtos.attach_ms(1000, &secondTick); // init eertos time manager
    _secondEERtos.attach_ms(10000, &tenSecondTick); // init eertos time manager
    // SetTask(TaskBlink1); // do blink
    SPIFFS.begin(); // Not really needed, checked inside library and started if
                    // needed

    // WiFi is started inside library
    ESPHTTPServer.begin(&SPIFFS);
    #if defined(ESP32)

	#endif
    Serial.print("*** Ep8266 service chip firmware ver: ");
    Serial.print(VERSION_APP);
    Serial.println(" ***");

    Serial.print("*** Ep8266 web pages ver: ");
    Serial.print(VERSION_WEB);
    Serial.println(" ***");

    Serial.print("*** build DateTime: ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.print(__TIME__);
    Serial.println(" ***");

    TerminalInit();

    setup_meteo();
}

void loop() {
    TaskManager();
    TerminalLoop();
    ESPHTTPServer.handle();
}

void usecondTick()  {
    TimerService();
}


void secondTick() {
   metar_loop();
}

void tenSecondTick() {
    update_weather();
}