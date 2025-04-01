// Standard C libraries
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ESP-IDF logging and system
#include "esp_log.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_sntp.h"
#include "esp_heap_caps.h"
#include "esp_crt_bundle.h"

// MQTT
#include "mqtt_client.h"

#define TAG "MQTT_MODULE"

// MQTT variables definition
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

// Variables definition with macros
static char mqtt_clientId[MQTT_CLIENT_ID_MAX_LEN] = MQTT_CLIENT_ID;
static char mqtt_publishTopic[MQTT_TOPIC_MAX_LEN] = MQTT_PUBLISH_TOPIC;
static char mqtt_responseTopic[MQTT_TOPIC_MAX_LEN] = MQTT_RESPONSE_TOPIC;
static char mqtt_subscribeTopic[MQTT_TOPIC_MAX_LEN] = MQTT_SUBSCRIBE_TOPIC;
static char mqtt_lastWill[MQTT_LAST_WILL_MAX_LEN] = MQTT_LAST_WILL;

static int mqtt_qos = 0;                                                        // QoS (Quality of Service)
static int mqtt_keepalive = 120;                                                // Keepalive time
static char mqtt_password[MQTT_PASSWORD_MAX_LEN] = "";                          // MQTT password
static char mqtt_url[MQTT_URL_MAX_LEN] = "";                                    // MQTT server URL
static char mqtt_userName[MQTT_USER_NAME_MAX_LEN] = "";                         // MQTT username

esp_mqtt_client_handle_t mqtt_client;

static inline bool is_mqtt_connected(void)
{
    if (!mqtt_client)
    {
        return false;
    }
    else {
        return true;
    }
    // if (!mqtt_connected)
    // {
    //     ESP_LOGW("MQTT", "No conectado al broker MQTT");
    // }
    // return mqtt_connected;
}

void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE("UTILS_MODULE", "Error %s: 0x%x", message, error_code);
    }
}

void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event base=%s, event_id=%" PRIi32, base, event_id);

    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_%s, msg_id=%d",
                 (event_id == MQTT_EVENT_SUBSCRIBED) ? "SUBSCRIBED" :
                 (event_id == MQTT_EVENT_UNSUBSCRIBED) ? "UNSUBSCRIBED" :
                 (event_id == MQTT_EVENT_PUBLISHED) ? "PUBLISHED" : "UNKNOWN",
                 event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, event->topic, event->topic_len, ESP_LOG_INFO);
        ESP_LOG_BUFFER_HEX_LEVEL(TAG, event->data, event->data_len, ESP_LOG_INFO);  

        ESP_LOGI(TAG, "MQTT Message recieved: %.*s", event->topic_len, event->data);
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("esp-tls error", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("TLS stack error", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("Socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGE(TAG, "Socket errno string: %s", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    case MQTT_EVENT_ANY:
        ESP_LOGI(TAG, "MQTT_EVENT_PINGRESP"); // check this out later
        break;

    default:
        ESP_LOGW(TAG, "Unhandled event id: %d", event->event_id);
        break;
    }
}

static void mqtt_init(void)
{
    // Config MQTT client
    esp_mqtt_client_config_t mqtt_config = {
    };

    ESP_LOGI(TAG, "Iniciando cliente MQTT con URL: %s", mqtt_url);

    mqtt_client = esp_mqtt_client_init(&mqtt_config);

    if (!mqtt_client)
    {
        ESP_LOGE(TAG, "Error when init MQTT client");
        return;
    }

    // Register MQTT events
    esp_err_t err = esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error when register MQTT event: %s", esp_err_to_name(err));
        return;
    }

    // Init MQTT client
    err = esp_mqtt_client_start(mqtt_client);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error when init MQTT: %s", esp_err_to_name(err));
        return;
    }

    // Registrar eventos MQTT
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error when register MQTT event: %s", esp_err_to_name(err));
        return;
    }
}

// void app_main(void)
// {

//     // esp_mqtt_client_subscribe_single(mqtt_client, mqtt_subscribeTopic, 0);

//     // esp_mqtt_client_publish(mqtt_client, mqtt_publishTopic, json_buffer, 0, 0, 0);
    
// }