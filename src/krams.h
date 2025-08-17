#ifndef __KRAMS_H__
#define __KRAMS_H__

#include <Arduino.h>

#define CODE_0_PIN 15
#define CLOCK_PIN 12
#define PREP_PIN 13


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

void metar_loop();
void setup_meteo();
void send_message(const message_t& message);
void send_code(uint8_t code);
void convert_data(const data_t& data, message_t& _message);
void convert_temp(int8_t temp, dword_t& code);
void convert_dword(int16_t data, dword_t& code);
void convert_word(int8_t data, word_t& code);
uint8_t convert_byte(int8_t data);

#endif // __KRAMS_H__