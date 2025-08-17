#include "weather.h"


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