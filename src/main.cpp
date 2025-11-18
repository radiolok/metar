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


#define CONFIG_FILE_METAR   "/config_metar.json"

std::shared_ptr<Metar> metar_ptr;

String icao = "UWGG";

bool save_config_metar(){
     JsonDocument jsonDoc;
    jsonDoc["icao"] = icao;
    return ESPHTTPServer.save_jsonDoc(jsonDoc, CONFIG_FILE_METAR);
}

bool load_config_metar() {
	JsonDocument jsonDoc;
	if (!ESPHTTPServer.load_jsonDoc(CONFIG_FILE_METAR, jsonDoc)){
		return false;
	}
	icao = jsonDoc["icao"].as<const char *>();
	Serial.print("load_config_metar:" );
	Serial.println(icao);
	return true;
}

void default_config_metar() {
	Serial.println("default_config_metar");
	icao 		= "UWGG";
	save_config_metar();
	DEBUGLOG(__PRETTY_FUNCTION__);
	DEBUGLOG("\r\n");
}

void handle_metar_html(AsyncWebServerRequest *request){
    if (request->args() > 0) { // Save Settings
		for (uint8_t i = 0; i < request->args(); i++) {
			DEBUGLOG("Arg %d: %s %s\r\n", i, request->argName(i).c_str() ,request->arg(i).c_str() );
			if (request->argName(i) == "icao") 		{
                icao = ESPHTTPServer.urldecode(request->arg(i));
                save_config_metar();
                continue;
            }
		}
	}
	ESPHTTPServer.handleFileRead(request->url(), request);
}

void handle_metar_ajax(AsyncWebServerRequest *request){
    String values = "";
    values += "icao|" 	  		+ icao 	+ "|select\n";
    request->send(200, "text/plain", values);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    InitRTOS(); // init eertos
    _secondEERtos.attach_ms(1, &usecondTick); // init eertos time manager
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


	if (!load_config_metar()) {
        default_config_metar();
    }

    setup_meteo();

    ESPHTTPServer.on("/metar.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        handle_metar_html(request);
	});

    ESPHTTPServer.on("/metar/info", HTTP_GET, [](AsyncWebServerRequest *request) {
        handle_metar_ajax(request);
	});
}

void loop() {
    TaskManager();
    TerminalLoop();
    ESPHTTPServer.handle();
}

void usecondTick()  {
    TimerService();
}

void tenSecondTick() {
    metar_ptr = update_weather(icao);
    if (metar_ptr){
        metar_loop(metar_ptr);
   }
}
