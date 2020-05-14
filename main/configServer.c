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
#include "captdns.h"

static const char* LOG_TAG = "ConfigServer";

static const char* INDEX_CONTENT =
	"<html>"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"</head>"
	"<body>"
	"Skonfiguruj swój sensor Szpek:<br>"
	"Configure your Szpek sensor:<br>"
	"<form action=\"config\" method=\"post\" style=\"margin-top: 1rem;\">"
	"<div style=\"margin-top: 1rem;\">Nazwa sieci (SSID):<br><input type=\"text\" name=\"ssid\"></div>"
	"<div style=\"margin-top: 1rem;\">Hasło (Password):<br><input type=\"text\" name=\"password\"></div>"
	"<input style=\"margin-top: 1rem; font-size: 1.1rem\" type=\"submit\" value=\"Zapisz (Save)\">"
	"</form>"
	"</body>"
	"</html>";

static const char* CONFIG_CONTENT =
	"<html>"
	"<head>"
	"<meta charset=\"UTF-8\">"
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
	"</head>"
	"<body>"
	"<div>"
	"Konfiguracja zapisana pomyślnie!<br>"
	"Teraz przytrzymaj przycisk na urządzeniu aż zgaśnie czerwona dioda.<br>"
	"Urządzenie rozpocznie łaczenie się ze skonfigurowaną siecią WiFi.<br>"
	"Pomyślne połączenie zostanie zasygnalizowane zapaleniem zielonej diody.<br>"
	"</div>"
	"<div style=\"margin-top: 1rem;\">"
	"Configuration saved successfully!<br>"
	"Now hold the button until the red LED goes out.<br>"
	"The device will begin connecting to configured WiFi.<br>"
	"Successfull connection will be indicated bythe greed LED lighting up.<br>"
	"</div>"
	"</body>"
	"</html>";

static httpd_handle_t s_serverHandle;
static esp_err_t rootGETHandler(httpd_req_t* request);
static esp_err_t configGETHandler(httpd_req_t* request);
static esp_err_t configPOSTHandler(httpd_req_t* request);

static esp_err_t notFoundHandler(httpd_req_t *req, httpd_err_code_t error);

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

static bool s_captDnsRunning = false;

bool configServerStart()
{
	LOG_INFO("Starting...");

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.lru_purge_enable = true;
	esp_err_t err = httpd_start(&s_serverHandle, &config);
	if (err != ESP_OK)
	{
		LOG_ERROR("An error occurred while starting HTTP server for configuration");
		return false;
	}

	httpd_register_uri_handler(s_serverHandle, &s_rootGET);
	httpd_register_uri_handler(s_serverHandle, &s_configGET);
	httpd_register_uri_handler(s_serverHandle, &s_configPOST);
	httpd_register_err_handler(s_serverHandle, HTTPD_404_NOT_FOUND, notFoundHandler);

	LOG_INFO("Started!");

	if (!s_captDnsRunning)
	{
		LOG_INFO("Starting captdns...");
		captdnsInit();
		s_captDnsRunning = true;
	}

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
	// TODO: check request uri and redirect to change url in the browser

	httpd_resp_send(request, INDEX_CONTENT, strlen(INDEX_CONTENT));

    return ESP_OK;
}

static esp_err_t configGETHandler(httpd_req_t* request)
{
	httpd_resp_set_status(request, "302 Found");
	httpd_resp_set_hdr(request, "Location", "http://config.szpek.pl");
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

static esp_err_t notFoundHandler(httpd_req_t* request, httpd_err_code_t error)
{
	LOG_INFO("Redirecting to main page");
	httpd_resp_set_status(request, "302 Found");
	httpd_resp_set_hdr(request, "Location", "http://config.szpek.pl");
	httpd_resp_send(request, NULL, 0);

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
