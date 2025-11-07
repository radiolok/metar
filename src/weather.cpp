#include "weather.h"

static std::shared_ptr<Metar> metar_ptr;

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
std::shared_ptr<Metar> update_weather(const String& icao_code) {
    String metar_str = get_metar_data(icao_code);

    if (!metar_str.isEmpty()) {
        Serial.println("METAR data received:");
        Serial.println(metar_str);

        // Здесь можно парсить данные
        // Пример: первая строка - дата, вторая - собственно METAR
        int newlinePos = metar_str.indexOf('\n');
        if (newlinePos != -1) {
            String observation_time = metar_str.substring(0, newlinePos);
            String metar_text = metar_str.substring(newlinePos + 1);

            Serial.println("Observation time: " + observation_time);
            Serial.println("METAR: " + metar_text);
        }

        metar_ptr = Metar::Create(metar_str.c_str());
        if (metar_ptr) {
            Serial.println("METAR parsed successfully");
        }
        return metar_ptr;
    } else {
        Serial.println("Failed to get METAR data");
    }
    return 0;
}