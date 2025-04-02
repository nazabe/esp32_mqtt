#ifndef ESP32_MQTT_H_INCLUDED
#define ESP32_MQTT_H_INCLUDED


#include "mqtt_client.h"
#include <stdbool.h>
#include "esp_err.h"

#define MQTT_CLIENT_ID "default"
#define MQTT_CLIENT_ID_MAX_LEN 64
#define MQTT_LAST_WILL_MAX_LEN 128
#define MQTT_TOPIC_MAX_LEN 128
#define MQTT_URL_MAX_LEN 128
#define MQTT_USER_NAME_MAX_LEN 64
#define MQTT_PASSWORD_MAX_LEN 64
#define MQTT_PUBLISH_TOPIC  ""
#define MQTT_RESPONSE_TOPIC ""
#define MQTT_SUBSCRIBE_TOPIC ""
#define MQTT_LAST_WILL "{\"status\" : \"" MQTT_CLIENT_ID " disconnected!\"}"

extern esp_mqtt_client_handle_t mqtt_client;

bool is_mqtt_connected(void);
void log_error_if_nonzero(const char *message, int error_code);
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_init(void);

#endif /* ESP32_MQTT_H_INCLUDED */