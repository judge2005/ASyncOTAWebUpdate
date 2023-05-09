/*
 * ASyncOTAWebUpdate.cpp
 *
 *  Created on: Nov 19, 2019
 *      Author: mpand
 */

#include <ASyncOTAWebUpdate.h>
#ifdef ESP32
#include "SPIFFS.h"
#else
extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;
#endif

#ifndef U_SPIFFS
#define U_SPIFFS U_FS
#endif

ASyncOTAWebUpdate::~ASyncOTAWebUpdate() {
}

void ASyncOTAWebUpdate::init(AsyncWebServer &server, const char *path, pUpdateFormFunc updateFormCallback, pUpdatingFunc updatingCallback) {
	this->updateFormCallback = updateFormCallback;
	this->updatingCallback = updatingCallback;
	// Dontcha just love C++?
	using namespace std::placeholders;
	server.on(path, HTTP_GET,
			std::bind(&ASyncOTAWebUpdate::handleUpdateGet, this, _1)
	).setFilter(ON_STA_FILTER);
	server.on(path, HTTP_POST,
			std::bind(&ASyncOTAWebUpdate::handleUpdatePost, this, _1),
			std::bind(&ASyncOTAWebUpdate::handleUpdateUpload, this, _1, _2, _3, _4, _5, _6)
	).setFilter(ON_STA_FILTER);
#ifdef ESP32
	Update.onProgress(std::bind(&ASyncOTAWebUpdate::printProgress, this, _1, _2));
#endif
}

void ASyncOTAWebUpdate::printProgress(size_t prg, size_t sz) {
#ifdef ESP32
	delay(1);
#endif
	if (_printProgress) {
		if (contentLen != 0) {
			Serial.printf("Progress: %d%%\n", (prg*100)/contentLen);
		} else {
			Serial.printf("Progress: %d of %d\n", prg, sz);
		}
	}
}

void ASyncOTAWebUpdate::handleUpdateGet(AsyncWebServerRequest *request) {
	if(!request->authenticate(username, password)) {
	    return request->requestAuthentication();
	}
	updateFormCallback(request);
}

void ASyncOTAWebUpdate::handleUpdatePost(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    updatingCallback(response, update.hasError());
    request->client()->onDisconnect([](void *arg, AsyncClient *client){
        ESP.restart();
    });
    request->send(response);
}

void ASyncOTAWebUpdate::handleUpdateUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
	contentLen = 0;

	if (!request->authenticate(username, password)) {
		return request->requestAuthentication();
	}

	if (!index) {
		if (_printProgress) Serial.println(F("update"));
		contentLen = request->contentLength();
		// if filename includes spiffs, update the spiffs partition
		int cmd = U_FLASH;
		if (filename.indexOf("spiffs") > -1) {
			cmd = U_SPIFFS;
			SPIFFS.end();
#ifdef ESP8266
			contentLen = ((size_t) &_SPIFFS_end - (size_t) &_SPIFFS_start);
#endif
		}

		if (_printProgress) Serial.printf("Loading %s\n", filename.c_str());

#ifdef ESP8266
		update.runAsync(true);
		if (!update.begin(contentLen, cmd)) {
#else
		if (!update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
#endif
			update.printError(Serial);
		}
	}

	if (update.write(data, len) != len) {
		update.printError(Serial);
#ifdef ESP8266
	} else {
		if (_printProgress) Serial.printf("Progress: %d%%\n", (update.progress()*100)/update.size());
#endif
	}

	if (final) {
		if (!update.end(true)) {
			update.printError(Serial);
		}
	}
}


