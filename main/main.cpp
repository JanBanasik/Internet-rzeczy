/* main.cpp */

// Nagłówki C++
#include <cstring> // dla std::memcpy

// Nagłówki C (ESP-IDF)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h" // Użyjemy grup zdarzeń zamiast boola
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

// --- Konfiguracja ---
#define WIFI_SSID "Redmi Note 10 Pro"
#define WIFI_PASS "banan111"
#define BLINK_GPIO     (gpio_num_t)2 // Wbudowana dioda LED

static const char *TAG = "WIFI_STA";

// Użyjemy grup zdarzeń FreeRTOS zamiast prostej flagi 'volatile bool'
// To bardziej idiomatyczne dla ESP-IDF i bezpieczniejsze wielowątkowo.
static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0; // Bit flagi oznaczający połączenie

// --- Funkcja obsługi zdarzeń WiFi ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect(); // Rozpocznij łączenie po starcie interfejsu
            ESP_LOGI(TAG, "Station started, connecting...");
        } 
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            // (Wymaganie 4) Ustawienie flagi braku połączenia
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            ESP_LOGW(TAG, "WiFi disconnected. Retrying...");
            
            // (Wymaganie 5) Ponowna próba połączenia
            // Domyślna konfiguracja ESP-IDF automatycznie próbuje połączyć się ponownie.
            // Jeśli chcesz ręcznie: esp_wifi_connect();
        }
    } 
    else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            
            // (Wymaganie 4) Ustawienie flagi połączenia
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }
}

// --- Funkcja inicjalizacji WiFi ---
void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    // Inicjalizacja stosu TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());

    // Inicjalizacja domyślnej pętli zdarzeń
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Utworzenie domyślnego interfejsu WiFi (Station)
    esp_netif_create_default_wifi_sta();

    // Inicjalizacja WiFi z domyślną konfiguracją
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Rejestracja handlerów zdarzeń (to jest odpowiednik WiFi.onEvent() z Arduino)
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    // Konfiguracja WiFi (SSID, hasło, itd.)
    wifi_config_t wifi_config = {}; // Zero-initialize struct
    std::memcpy(wifi_config.sta.ssid, WIFI_SSID, sizeof(WIFI_SSID));
    std::memcpy(wifi_config.sta.password, WIFI_PASS, sizeof(WIFI_PASS));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    // (Wymaganie 1) Ustawienie trybu Station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    
    // Uruchomienie WiFi
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

// --- Task (zadanie) do mrugania diodą ---
// (Wymaganie 3)
void blink_task(void *pvParameter)
{
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    bool led_state = false;

    while (1)
    {
        // Sprawdź flagę połączenia (bit w grupie zdarzeń)
        EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
        bool isConnected = (bits & WIFI_CONNECTED_BIT) != 0;

        if (isConnected) {
            // Połączono -> dioda świeci na stałe
            gpio_set_level(BLINK_GPIO, 1);
            vTaskDelay(500 / portTICK_PERIOD_MS); // Sprawdzaj stan rzadziej
        } else {
            // Brak połączenia -> mruganie
            led_state = !led_state;
            gpio_set_level(BLINK_GPIO, (uint32_t)led_state);
            vTaskDelay(250 / portTICK_PERIOD_MS); // Szybkie mruganie
        }
    }
}

// --- Główna funkcja programu ---
extern "C" {
    void app_main(void);
}

void app_main(void)
{
    // Inicjalizacja NVS (Non-Volatile Storage) - wymagane przez WiFi
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // Uruchomienie zadania do mrugania diodą
    xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);
}