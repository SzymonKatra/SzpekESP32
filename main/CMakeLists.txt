idf_component_register(SRCS "main.c"
							"network.c"
							"app.c"
							"tasks/taskSmogSensor.c"
							"tasks/taskPushReports.c"
							"tasks/taskAggregateData.c"
							"tasks/taskCheckButton.c"
							"tasks/taskFirmwareOTA.c"
							"tasks/taskWeatherSensor.c"
							"tasks/taskLogs.c"
							"timers/timerTimeTriggers.c"
							"smog.c"
							"weather.c"
							"crypto.c"
							"settings.c"
							"szpekApi/v1/szpekApiV1.c"
							"configServer.c"
							"timeTriggers.c"
							"captdns.c"
							"logUtils.c"
                    INCLUDE_DIRS "."
                    )