/*
 * app.h
 *
 *  Created on: 14 mar 2020
 *      Author: szymo
 */

#ifndef MAIN_APP_H_
#define MAIN_APP_H_

#include <esp_event.h>

void appInit();

esp_event_loop_handle_t appGetEventLoopHandle();


#endif /* MAIN_APP_H_ */
