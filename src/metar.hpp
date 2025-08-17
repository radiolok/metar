#ifndef __METAR_H__
#define __METAR_H__

#include <string.h>
#include <stdlib.h>

// Определение платформы
#ifdef ARDUINO
#include <Arduino.h>
#define METAR_PLATFORM_ESP32 1
#else
#define METAR_PLATFORM_WINDOWS 1
#endif

// Максимальное количество токенов в METAR
#define MAX_TOKENS 20
// Максимальная длина одного токена
#define MAX_TOKEN_LEN 16

// Максимальная длина для компонентов ветра
#define WIND_DIR_LEN 4    // 3 символа + \0
#define WIND_SPEED_LEN 4  // 3 символа + \0 (обычно 2-3 цифры)

typedef struct {
    char day[2];     // День месяца (1-31)
    char hour[2];    // Часы (0-23)
    char minute[2];  // Минуты (0-59)
} MetarTime;

// Коды погодных явлений (обновленные)
typedef enum {
    WEATHER_NONE = 0,   // Нет явлений
    WEATHER_FOG,        // Туман (1)
    WEATHER_HAZE,       // Дымка (2)
    WEATHER_HAIL,       // Град (3)
    WEATHER_RAIN,       // Дождь (4)
    WEATHER_SNOW,       // Снег (5)
    WEATHER_DRIFTING,   // Поземок (6)
    WEATHER_BLIZZARD,   // Метель (7)
    WEATHER_THUNDER     // Гроза (8) - теперь только для основного поля
} WeatherCode;

typedef struct {
    char station[8];        // Код станции
    MetarTime observation;  // Время наблюдения
    char wind_dir[4];       // Направление ветра
    char wind_speed[4];     // Скорость ветра
    char general[5];    // Общая видимость (4 символа + \0)
    char runway[5];     // Видимость на ВПП (4 символа + \0)
    WeatherCode weather;    // Основное явление (0-8)
    bool is_thunder;        // Флаг грозы
    bool is_icing;          // Флаг гололеда
    bool is_complex;        // Флаг сложных условий
    // ... другие поля ...
} MetarData;

// Функция для кроссплатформенного вывода
void metar_printf(const char* format, ...);
int tokenize_metar(const char* raw, char tokens[][MAX_TOKEN_LEN], int max_tokens) ;
void process_metar(const char* raw_metar);
bool process_tokens(char tokens[][MAX_TOKEN_LEN], int token_count, MetarData* out);
bool parse_metar_time(const char* token, MetarTime* out_time) ;
bool extract_wind_components(const char* token, char* dir_buf, char* speed_buf) ;

#endif