#include <Arduino.h>

#if defined(ESP32)
#include <SPIFFS.h>
#elif defined(ESP8266)
#include <FS.h>
#endif
#include <Ticker.h>
#include <ESPAsyncWebServer.h>
#include "main.h"
#include "FSWebServerLib.h"
#include "eertos.h"
#include "ErriezSerialTerminal.h"
#include "terminal.h"
#include "avrisp.h"
#include "udphelper.h"

#define CODE_0_PIN 15
#define CLOCK_PIN 12
#define PREP_PIN 13

// pin used for entering setup mode

 Ticker _secondEERtos;
void usecondTick();
void secondTick();


// unsigned long previousMillis = 0;
// unsigned long interval = 10000;

void setup_meteo() {
    pinMode(CODE_0_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(PREP_PIN, OUTPUT);
}

uint8_t codes[] = {
    0x12, //0
    0x02, //1
    0x06, //2
    0x0F, //3
    0x15, //4
    0x1E, //5
    0x0A, //6
    0x03, //7
    0x13, //8
    0x1C, //9
    0x07, //-
    0x1B //space
};

typedef struct {
    uint8_t high;
    uint8_t low;
}    word_t;


typedef struct {
    word_t low;
    word_t high;
}    dword_t;

//Here we store raw data!
typedef struct {
    int8_t hours;       //1 - 2
    int8_t minutes;     //3 - 4
    int8_t wind_dir;    //5 - 6
    int8_t wind_speed;  //7 - 8
    int8_t wind_max;    //9 - 10
    int8_t rwy_max_speed; //11 -12
    int8_t clouds;     //13
    int8_t clouds_low_level;//14
    int8_t clouds_form; //15 - 16
    int16_t clouds_heigth; //17 - 20
    int16_t pressure_torr; //21-24
    int16_t distanse_meteo;//25-28
    int16_t distanse_l1;//29-32
    int16_t distanse_l2; //33-36
    int16_t distanse_l3; //37-40
    int16_t temperature; //41-44
    int16_t humidity;   //45-48
    int16_t pressure_mbar;//49-52
    int8_t forecast;   //53
    int8_t telegram_name; //54
    int16_t number_bd;//55-56
    int8_t bi_thunder;//57
    int8_t bi_ice;//58
    int8_t rta_thunder;//59
    int8_t rta_ice;//60
    int16_t rsvd_1;//61-64
} data_t;

//Here we already store codes!
typedef struct {
    word_t hours;       //1 - 2
    word_t minutes;     //3 - 4
    word_t wind_dir;    //5 - 6
    word_t wind_speed;  //7 - 8
    word_t wind_max;    //9 - 10
    word_t rwy_max_speed; //11 -12
    uint8_t clouds;     //13
    uint8_t clouds_low_level;//14
    word_t clouds_form; //15 - 16
    dword_t clouds_heigth; //17 - 20
    dword_t pressure_torr; //21-24
    dword_t distanse_meteo;//25-28
    dword_t distanse_l1;//29-32
    dword_t distanse_l2; //33-36
    dword_t distanse_l3; //37-40
    dword_t temperature; //41-44
    dword_t humidity;   //45-48
    dword_t pressure_mbar;//49-52
    uint8_t forecast;   //53
    uint8_t telegram_name; //54
    word_t number_bd;//55-56
    uint8_t bi_thunder;//57
    uint8_t bi_ice;//58
    uint8_t rta_thunder;//59
    uint8_t rta_ice;//60
    dword_t rsvd_1;//61-64
} message_t;

uint8_t convert_byte(int8_t data){
    if (data < 0) {
        return codes[10];//-
    }
    if (data < 10) {
        return codes[data];//digit
    }
    return codes[11];//space
}

void convert_word(int8_t data, word_t& code){
    code.high = convert_byte(data % 10);
    code.low = convert_byte(data / 10);
}

void convert_dword(int16_t data, dword_t& code){
    convert_word(data % 100, code.high);
    convert_word(data / 100, code.low);
}

void convert_temp(int8_t temp, dword_t& code){
    uint8_t temp_abs = (temp < 0) ? -temp : temp;
    code.high.low = convert_byte(temp_abs % 10);
    code.low.high = convert_byte(temp_abs / 10);
    code.low.low = (temp < 0 ) ? convert_byte(-1) : convert_byte(11);
}

void convert_data(const data_t& data, message_t& _message){
    convert_word(data.hours, _message.hours);
    convert_word(data.minutes, _message.minutes);
    convert_word(data.wind_dir, _message.wind_dir);
    convert_word(data.wind_speed, _message.wind_speed);
    convert_word(data.wind_max, _message.wind_max);
    convert_word(data.rwy_max_speed, _message.rwy_max_speed);
    _message.clouds = convert_byte(data.clouds);
    _message.clouds_low_level = convert_byte(data.clouds_low_level);
    convert_word(data.clouds_form, _message.clouds_form);
    convert_dword(data.clouds_heigth, _message.clouds_heigth);
    convert_dword(data.pressure_torr, _message.pressure_torr);
    convert_dword(data.distanse_meteo, _message.distanse_meteo);
    convert_dword(data.distanse_l1, _message.distanse_l1);
    convert_dword(data.distanse_l2, _message.distanse_l2);
    convert_dword(data.distanse_l3, _message.distanse_l3);
    convert_temp(data.temperature, _message.temperature);
    convert_dword(data.humidity, _message.humidity);
    convert_dword(data.pressure_mbar, _message.pressure_mbar);
    _message.forecast = convert_byte(data.forecast);
    _message.telegram_name = convert_byte(data.telegram_name);
    convert_word(data.number_bd, _message.number_bd);
    _message.bi_thunder = convert_byte(data.bi_thunder);
    _message.bi_ice = convert_byte(data.bi_ice);
    _message.rta_thunder = convert_byte(data.rta_thunder);
    _message.rta_ice = convert_byte(data.rta_ice);
}

inline void send_one(){
    digitalWrite(CLOCK_PIN, HIGH);
    delay(1);
    digitalWrite(CLOCK_PIN, LOW);
    delay(1);
}

inline void send_zero(){
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CODE_0_PIN, HIGH);
    delay(1);
    digitalWrite(CODE_0_PIN, LOW);
    digitalWrite(CLOCK_PIN, LOW);
    delay(1);
}

void send_code(uint8_t code) {
    //start bit:
    send_one();
    //Five data bits from lowest one:
    for (uint8_t i = 0; i < 5; ++i){
        uint8_t bit = (code >> i) & 0x01;
        if (bit){
            send_one();
        } else {
            send_zero();
        }
    }
    //stop bit:
    send_zero();
}

void send_message(const message_t& message) {
    digitalWrite(PREP_PIN, HIGH);
    delay(5);
    digitalWrite(PREP_PIN, LOW);
    delay(1);
    uint8_t* data = (uint8_t*)&message;
    for (uint8_t i = 0; i < sizeof(message_t); ++i){
        send_code(*(data + i));
    }
}

data_t metar_data;
message_t metar_message;

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
    loop_user();
    TerminalLoop();
    ESPHTTPServer.handle();

}

void loop_user(){

}


void usecondTick()  {
    TimerService();
}

void secondTick() {
    uint8_t* data = (uint8_t*)&metar_data;
    for (uint8_t i = 0; i < sizeof(data_t); ++i){
        data[i] = i % 10;
    }
    convert_data(metar_data, metar_message);
    send_message(metar_message);
}