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
#include <WiFiClient.h>
#include <HTTPClient.h>

#include "krams.h"

Ticker _secondEERtos;
void usecondTick();
void secondTick();

String get_metar_data(const String& icao_code) {
    // Формируем URL запроса
    String url = "https://tgftp.nws.noaa.gov/data/observations/metar/stations/" + icao_code + ".TXT";

    WiFiClientSecure client;
    HTTPClient http;
    String payload = "";

    // Отключаем проверку сертификата (INSECURE MODE)
    client.setInsecure();

    // Начинаем HTTP-соединение
    if (http.begin(client, url)) {
        // Выполняем GET-запрос
        int httpCode = http.GET();

        // Проверяем код ответа
        if (httpCode == HTTP_CODE_OK) {
            payload = http.getString();
        } else {
            Serial.printf("[HTTP] GET failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        // Закрываем соединение
        http.end();
    } else {
        Serial.println("[HTTP] Unable to connect");
    }

    return payload;
}

// Пример использования в коде:
void update_weather() {
    String metar = get_metar_data("UWGG");

    if (!metar.isEmpty()) {
        Serial.println("METAR data received:");
        Serial.println(metar);

        // Здесь можно парсить данные
        // Пример: первая строка - дата, вторая - собственно METAR
        int newlinePos = metar.indexOf('\n');
        if (newlinePos != -1) {
            String observation_time = metar.substring(0, newlinePos);
            String metar_text = metar.substring(newlinePos + 1);

            Serial.println("Observation time: " + observation_time);
            Serial.println("METAR: " + metar_text);
        }
    } else {
        Serial.println("Failed to get METAR data");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    InitRTOS(); // init eertos
    _secondEERtos.attach_ms(1, &usecondTick); // init eertos time manager
    _secondEERtos.attach_ms(1000, &secondTick); // init eertos time manager
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