
#define CODE_0_PIN 15
#define CLOCK_PIN 12
#define PREP_PIN 13
#define LINE_PIN 14

#define DELAY_US (20)


#include <map>
#include <Phenom.h>
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
    0x07 //-
};

void setup_meteo() {
    pinMode(CODE_0_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(PREP_PIN, OUTPUT);
    pinMode(LINE_PIN, OUTPUT);
    digitalWrite(LINE_PIN, LOW);
}

uint8_t convert_byte(int8_t data){
    if (data < 0) {
        return codes[10];//-
    }
    if (data < 10) {
        return codes[data];//digit
    }
    return codes[10];//-
}

void convert_word(int8_t data, word_t& code){
    if (data < 0 ) {
        memset(&code, codes[10], sizeof(code));
    } else {
        code.high = convert_byte(data % 10);
        code.low = convert_byte((data / 10) % 10);
    }
}

void convert_tword(int16_t data, tword_t& code){
    if (data < 0 ) {
        memset(&code, codes[10], sizeof(code));
    } else {
        code.high = convert_byte(data % 10);
        convert_word(data / 10, code.low);
    }
}

void convert_dword(int16_t data, dword_t& code){
    if (data < 0 ) {
        memset(&code, codes[10], sizeof(code));
    } else {
        convert_word(data % 100, code.high);
        convert_word(data / 100, code.low);
    }
}

void convert_temp(int8_t temp, tword_t& code){
    uint8_t temp_abs = (temp < 0) ? -temp : temp;
    code.high = convert_byte(temp_abs % 10);
    code.low.high = convert_byte(temp_abs / 10);
    code.low.low = (temp < 0 ) ? convert_byte(1) : convert_byte(10);
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
    convert_tword(data.clouds_heigth, _message.clouds_heigth);
    convert_tword(data.pressure_torr, _message.pressure_torr);
    convert_tword(data.distanse_meteo, _message.distanse_meteo);
    convert_tword(data.distanse_l1, _message.distanse_l1);
    convert_tword(data.distanse_l2, _message.distanse_l2);
    convert_tword(data.distanse_l3, _message.distanse_l3);
    convert_temp(data.temperature, _message.temperature);
    convert_tword(data.humidity, _message.humidity);
    convert_dword(data.pressure_mbar, _message.pressure_mbar);
    _message.forecast = convert_byte(data.forecast);
    _message.telegram_name = convert_byte(data.telegram_name);
    _message.number_bd = convert_byte(data.number_bd);
    _message.bi_thunder = convert_byte(data.bi_thunder);
    _message.bi_ice = convert_byte(data.bi_ice);
}

inline void send_one(){
    digitalWrite(CLOCK_PIN, HIGH);
    delayMicroseconds(DELAY_US*2);
    digitalWrite(CLOCK_PIN, LOW);
    delayMicroseconds(DELAY_US*1);
}

inline void send_zero(){
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CODE_0_PIN, LOW);
    delayMicroseconds(DELAY_US*2);
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(CODE_0_PIN, HIGH);
    delayMicroseconds(DELAY_US*1);
}

void send_code(uint8_t code) {
    //start bit:
    delayMicroseconds(DELAY_US*2);
    send_one();
    for (int i = 4; i >= 0; i--) {
        if (code & (1 << i)) {
            send_one();
        } else {
            send_zero();
        }
    }
    //stop bit:
    send_zero();
}

#define MESSAGE_LENGTH (47)

 std::map<Phenom::phenom, int> phenom2int =
  {
    {Phenom::phenom::NONE,             0},
    {Phenom::phenom::MIST,             2},
    {Phenom::phenom::DUST_STORM,       7},
    {Phenom::phenom::DUST,             7},
    {Phenom::phenom::DRIZZLE,          6},
    {Phenom::phenom::FUNNEL_CLOUD,     7},
    {Phenom::phenom::FOG,              1},
    {Phenom::phenom::SMOKE,            2},
    {Phenom::phenom::HAIL,             3},
    {Phenom::phenom::SMALL_HAIL,       3},
    {Phenom::phenom::HAZE,             2},
    {Phenom::phenom::ICE_CRYSTALS,     5},
    {Phenom::phenom::ICE_PELLETS,      5},
    {Phenom::phenom::DUST_SAND_WHORLS, 7},
    {Phenom::phenom::SPRAY,            4},
    {Phenom::phenom::RAIN,             4},
    {Phenom::phenom::SAND,             7},
    {Phenom::phenom::SNOW_GRAINS,      7},
    {Phenom::phenom::SHOWER,           4},
    {Phenom::phenom::SNOW,             5},
    {Phenom::phenom::SQUALLS,          4},
    {Phenom::phenom::SAND_STORM,       7},
    {Phenom::phenom::UNKNOWN_PRECIP,  7},
    {Phenom::phenom::VOLCANIC_ASH,     7}
  };

void send_message(const message_t& message) {
    Serial.printf("send_message\n");
    digitalWrite(CODE_0_PIN, LOW);
    delayMicroseconds(DELAY_US*2);
    digitalWrite(LINE_PIN, HIGH);
    delayMicroseconds(DELAY_US*20);
    digitalWrite(PREP_PIN, LOW);
    digitalWrite(CODE_0_PIN, HIGH);
    delayMicroseconds(DELAY_US*20);
    digitalWrite(PREP_PIN, HIGH);
    delayMicroseconds(DELAY_US*2);
    uint8_t* data = (uint8_t*)&message;
    for (uint8_t i = 0; i < MESSAGE_LENGTH; ++i){
        if (!(i % 10)) {
            Serial.printf("\n");
        }
        Serial.printf("%x ", *(data + i));
        send_code(*(data + i));
    }
    Serial.printf("\n");
    digitalWrite(LINE_PIN, LOW);
}

data_t metar_data;
message_t metar_message;

static void metar_to_krams(std::shared_ptr<Metar> _metar_ptr, data_t& _metar_data){
    _metar_data.hours = _metar_ptr->Hour().value_or(0);
    _metar_data.minutes = _metar_ptr->Minute().value_or(0);
    Serial.printf("Time: %d:%d\n", _metar_data.hours, _metar_data.minutes);
    float speedkoef = 1;
    Metar::speed_units units = _metar_ptr->WindSpeedUnits().value_or(Metar::speed_units::KPH);
    switch(units){
        case (Metar::speed_units::KT):
        speedkoef = 1.852;
            break;
        case (Metar::speed_units::KPH):
            speedkoef = 0.27;
            break;
    };
    _metar_data.wind_dir = _metar_ptr->WindDirection().value_or(0);
    _metar_data.wind_dir /= 10;
    _metar_data.wind_speed = _metar_ptr->WindSpeed().value_or(0) * speedkoef;
    Serial.printf("Wind: %dkm/h, Dir: %d\n", _metar_data.wind_speed, _metar_data.wind_dir);
    if (_metar_ptr->isVariableWindDirection()){
        _metar_data.wind_max = _metar_ptr->WindGust().value_or(0) * speedkoef;
        Serial.printf("Gust: %dkm/h\n", _metar_data.wind_max);
    } else {
        _metar_data.wind_max = -1;
    }
    _metar_data.distanse_meteo = _metar_ptr->Visibility().value_or(0) ;
    if (_metar_ptr->isCAVOK()){
        _metar_data.distanse_meteo = 9999;
    }
    Serial.printf("Wisibility: %d\n", _metar_data.distanse_meteo);
    _metar_data.distanse_meteo  = _metar_data.distanse_meteo  / 10;
    _metar_data.clouds_heigth = _metar_ptr->VerticalVisibility().value_or(-1);
    if (_metar_data.clouds_heigth > 0){
        _metar_data.clouds_heigth = _metar_data.clouds_heigth / 10;
    }
    Serial.printf("VerticalVisibility: %d\n", _metar_data.clouds_heigth);
    _metar_data.temperature = _metar_ptr->Temperature().value_or(0);
    Serial.printf("Temperature: %d\n", _metar_data.temperature);

    float Qpressure =  _metar_ptr->AltimeterQ().value_or(-1);
    float Apressure =  _metar_ptr->AltimeterA().value_or(-1);
    if (Qpressure > Apressure) {
        Apressure = Qpressure * 0.750062;
    } else {
        Qpressure = Apressure / 0.750062;
    }
    _metar_data.pressure_mbar = static_cast<int>(Qpressure);
    _metar_data.pressure_torr = static_cast<int>(Apressure);
    Serial.printf("Pressure: %d hPA, %d, torr\n", _metar_data.pressure_mbar, _metar_data.pressure_torr);

    _metar_data.distanse_l1 = -1;
    _metar_data.distanse_l2 = -1;
    _metar_data.distanse_l3 = -1;

    _metar_data.humidity = -1;

    _metar_data.clouds = _metar_ptr->NumCloudLayers();
    _metar_data.clouds_low_level = -1;

    if (_metar_ptr->NumPhenomena()) {
        const Phenom& p = _metar_ptr->Phenomenon(0);
        _metar_data.forecast = phenom2int[p[0]];
    } else {
        _metar_data.forecast = 0;
    }

    _metar_data.rwy_max_speed = -1;
    _metar_data.number_bd = -1;

}

void metar_loop(std::shared_ptr<Metar> metar_ptr) {
    metar_to_krams(metar_ptr, metar_data);
    convert_data(metar_data, metar_message);
    send_message(metar_message);
}


