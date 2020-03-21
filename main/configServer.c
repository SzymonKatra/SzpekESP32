/*
 * configServer.c
 *
 *  Created on: 20 mar 2020
 *      Author: szymo
 */

#include "configServer.h"

#include <esp_http_server.h>
#include "url_encoding.h"
#include "settings.h"
#include "logUtils.h"
#include "mathUtils.h"

static const char* LOG_TAG = "ConfigServer";

static const char* INDEX_CONTENT =
	"<html><body>"
	"Konfiguruj:"
	"<form action=\"config\" method=\"post\">"
	"SSID: <input type=\"text\" name=\"ssid\"><br>"
	"Haslo: <input type=\"text\" name=\"password\"><br>"
	"<input type=\"submit\" value=\"Zapisz\">"
	"</form>"
	"</body></html>";

static const char* CONFIG_CONTENT =
	"<html><body>"
	"Konfiguracja zapisana pomyslnie!<br>"
	"Teraz przytrzymaj przycisk az zgasnie czerwona dioda.<br>"
	"Urzadzenie rozpocznie laczenie sie z WiFi i zostanie to zasygnalizowane zapaleniem sie zielonej diody."
	"</body></html>";

static httpd_handle_t s_serverHandle;
static esp_err_t rootGETHandler(httpd_req_t* request);
static esp_err_t configGETHandler(httpd_req_t* request);
static esp_err_t configPOSTHandler(httpd_req_t* request);

static bool getFormValue(const char* buffer, const char* key, char* value, size_t len);

static const httpd_uri_t s_rootGET = {
	.uri       = "/",
	.method    = HTTP_GET,
	.handler   = rootGETHandler
};

static const httpd_uri_t s_configGET = {
	.uri	   = "/config",
	.method    = HTTP_GET,
	.handler   = configGETHandler
};

static const httpd_uri_t s_configPOST = {
	.uri       = "/config",
	.method    = HTTP_POST,
	.handler   = configPOSTHandler
};

bool configServerStart()
{
	LOG_INFO("Starting...");

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	esp_err_t err = httpd_start(&s_serverHandle, &config);
	if (err != ESP_OK)
	{
		LOG_ERROR("An error occurred while starting HTTP server for configuration");
		return false;
	}

	httpd_register_uri_handler(s_serverHandle, &s_rootGET);
	httpd_register_uri_handler(s_serverHandle, &s_configGET);
	httpd_register_uri_handler(s_serverHandle, &s_configPOST);

	LOG_INFO("Started!");

	return true;
}

void configServerStop()
{
	LOG_INFO("Stopping...");
	if (s_serverHandle != NULL)
	{
		httpd_stop(s_serverHandle);
		s_serverHandle = NULL;
		LOG_INFO("Stopeed!");
	}
}

static esp_err_t rootGETHandler(httpd_req_t* request)
{
	httpd_resp_send(request, INDEX_CONTENT, strlen(INDEX_CONTENT));

    return ESP_OK;
}

static esp_err_t configGETHandler(httpd_req_t* request)
{
	httpd_resp_set_status(request, "302 Found");
	httpd_resp_set_hdr(request, "Location", "/");
	httpd_resp_send(request, NULL, 0);

	return ESP_OK;
}

static esp_err_t configPOSTHandler(httpd_req_t* request)
{
	const size_t BUFFER_SIZE = 1024;
	char* buffer = malloc(BUFFER_SIZE);
	httpd_req_recv(request, buffer, min(request->content_len, BUFFER_SIZE - 1));
	buffer[request->content_len] = 0;

	settingsWifi_t wifiSettings = *settingsGetWifi();
	getFormValue(buffer, "ssid", wifiSettings.ssid, NETWORK_SSID_SIZE);
	getFormValue(buffer, "password", wifiSettings.password, NETWORK_PASSWORD_SIZE);
	settingsSetWifi(&wifiSettings);

	LOG_INFO("New SSID: %s", wifiSettings.ssid);
	LOG_INFO("New password: %s", wifiSettings.password);

	free(buffer);
	httpd_resp_send(request, CONFIG_CONTENT, strlen(CONFIG_CONTENT));

    return ESP_OK;
}

static bool getFormValue(const char* buffer, const char* key, char* value, size_t len)
{
	size_t paramBufferLen = len * 3;
	char* paramBuffer = malloc(paramBufferLen);
	bool result = false;

	if (httpd_query_key_value(buffer, key, paramBuffer, paramBufferLen) == ESP_OK)
	{
		urlDecode(paramBuffer, value, len);
		result = true;
	}

	free(paramBuffer);

	return result;
}
