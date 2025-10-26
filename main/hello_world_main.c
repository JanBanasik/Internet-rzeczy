/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
void hello_main(void)
{
    vTaskDelay(500 / portTICK_PERIOD_MS); // daj pół sekundy na start
    printf("Hello world from ESP-IDF in Wokwi!\n");
    fflush(stdout);

    for (int i = 5; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
