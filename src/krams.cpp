
#define CODE_0_PIN 15
#define CLOCK_PIN 12
#define PREP_PIN 13

#include "krams.h"

static uint8_t codes[] = {
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

void setup_meteo() {
    pinMode(CODE_0_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(PREP_PIN, OUTPUT);
}

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
    Serial.printf("convert_data\n");
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
    delay(4);
    digitalWrite(CLOCK_PIN, LOW);
    delay(2);
}

inline void send_zero(){
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CODE_0_PIN, HIGH);
    delay(4);
    digitalWrite(CODE_0_PIN, LOW);
    digitalWrite(CLOCK_PIN, LOW);
    delay(2);
}

void send_code(uint8_t code) {
    Serial.printf("%x ", code);
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
    Serial.printf("send_message\n");
    digitalWrite(PREP_PIN, HIGH);
    delay(30);
    digitalWrite(PREP_PIN, LOW);
    delay(4);
    uint8_t* data = (uint8_t*)&message;
    for (uint8_t i = 0; i < sizeof(message_t); ++i){
        send_code(*(data + i));
    }
}

data_t metar_data;
message_t metar_message;


static void metar_to_krams(std::shared_ptr<Metar> _metar_ptr, data_t& _metar_data){
     Serial.printf("Hour: ");
    _metar_data.hours = _metar_ptr->Hour().value();
    Serial.printf("%d\nMinute: ", _metar_data.hours);
    _metar_data.minutes = _metar_ptr->Minute().value();
    Serial.printf("%d\nWindDirection: ", _metar_data.minutes );
    _metar_data.wind_dir = _metar_ptr->WindDirection().value();
    Serial.printf("%d\nWindSpeed: ", _metar_data.wind_dir);
    _metar_data.wind_speed = _metar_ptr->WindSpeed().value();
    Serial.printf("%d\nVisibility: ", _metar_data.wind_speed);
    _metar_data.distanse_meteo = _metar_ptr->Visibility().value();
    Serial.printf("%d\n", _metar_data.distanse_meteo);
}

void metar_loop(std::shared_ptr<Metar> metar_ptr) {
    metar_to_krams(metar_ptr, metar_data);
    convert_data(metar_data, metar_message);
    send_message(metar_message);
}


