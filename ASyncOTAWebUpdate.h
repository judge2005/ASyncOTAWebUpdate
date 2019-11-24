/*
 * ASyncOTAWebUpdate.h
 *
 *  Created on: Nov 19, 2019
 *      Author: mpand
 */

#ifndef LIBRARIES_ASYNCOTAWEBUPDATE_ASYNCOTAWEBUPDATE_H_
#define LIBRARIES_ASYNCOTAWEBUPDATE_ASYNCOTAWEBUPDATE_H_

#include "ESPAsyncWebServer.h"
#ifdef ESP32
#include "Update.h"
#else
#include "Updater.h"
#define UpdateClass UpdaterClass
#endif

class ASyncOTAWebUpdate {
public:
	typedef void (*pUpdateFormFunc)(AsyncWebServerRequest *request);
	typedef void (*pUpdatingFunc)(AsyncResponseStream *response, boolean hasError);

	ASyncOTAWebUpdate(UpdateClass &update, const char *username, const char *password) :
		update(update), username(username), password(password)
	{
	}
	virtual ~ASyncOTAWebUpdate();
	void init(AsyncWebServer &server, const char *path, pUpdateFormFunc updateFormCallback, pUpdatingFunc updatingCallback);
	void setPrintProgress(boolean printProgress) { this->_printProgress = printProgress; }

private:
	boolean _printProgress = false;
	int contentLen;
	void handleUpdateGet(AsyncWebServerRequest *request);
	void handleUpdatePost(AsyncWebServerRequest *request);
	void handleUpdateUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
	void printProgress(size_t prg, size_t sz);

	UpdateClass &update;
	pUpdateFormFunc updateFormCallback;
	pUpdatingFunc updatingCallback;
	const char *username;
	const char *password;
};

#endif /* LIBRARIES_ASYNCOTAWEBUPDATE_ASYNCOTAWEBUPDATE_H_ */
