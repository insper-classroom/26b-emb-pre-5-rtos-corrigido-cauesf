/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

void btn_callback(uint gpio, uint32_t events) {
    if (events == 0x4) {
        if (gpio == BTN_PIN_R) {
            xSemaphoreGiveFromISR(xSemaphore_r, NULL);
        }

        if (gpio == BTN_PIN_Y) {
            xSemaphoreGiveFromISR(xSemaphore_y, NULL);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    int led_ativo = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphore_r, 0) == pdTRUE) {
            if (led_ativo == 0) {
                led_ativo = 1;
            } else {
                led_ativo = 0;
                gpio_put(LED_PIN_R, 0);
            }
        }

        if (led_ativo == 1) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(100));

            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    int led_ativo = 0;

    while (true) {
        if (xSemaphoreTake(xSemaphore_y, 0) == pdTRUE) {
            if (led_ativo == 0) {
                led_ativo = 1;
            } else {
                led_ativo = 0;
                gpio_put(LED_PIN_Y, 0);
            }
        }

        if (led_ativo == 1) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(100));

            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        } else {
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
}

void btn_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(
        BTN_PIN_R,
        GPIO_IRQ_EDGE_FALL,
        true,
        &btn_callback
    );

    gpio_set_irq_enabled(
        BTN_PIN_Y,
        GPIO_IRQ_EDGE_FALL,
        true
    );

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main() {
    stdio_init_all();

    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xTaskCreate(btn_task, "BTN_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(led_1_task, "LED_Task R", 256, NULL, 1, NULL);
    xTaskCreate(led_2_task, "LED_Task Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {
    }

    return 0;
}