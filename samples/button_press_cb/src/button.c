/*
 * Copyright (c) 2023 T-Mobile USA, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>

#include "button.h"

#define SW0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;
static button_press_callback_t registered_callback;
static struct gpio_callback button_cb_data;
button_cb_state button_state = BUTTON_CB_STATE_DEINIT;
struct k_timer debounce_timer;
static int64_t last_button_press_time;
static int64_t button_press_duration;
static int64_t max_button_press_timeout;

/* button GPIO interrupt handler */
static void button_press_handler(const struct device *port, struct gpio_callback *cb,
				 gpio_port_pins_t pins)
{
	k_timer_stop(&debounce_timer);
	k_timer_start(&debounce_timer, K_MSEC(50), K_NO_WAIT);
	button_state = BUTTON_CB_STATE_DEBOUNCING;
}

/* debounce timer handler */
static void button_debounce_timer_handler(struct k_timer *debounce_timer)
{
	if (BUTTON_CB_STATE_DEINIT == button_state) {
		return;
	}
	button_state = BUTTON_CB_STATE_DEBOUNCED;
	/* Each time the button is pressed, last_button_press_time will be 0,
	 * indicating a new action on user button */
	if (last_button_press_time == 0) {
		/* button press detected */
		last_button_press_time = k_uptime_get();
	} else if (registered_callback != NULL) {
		/* button release detected */
		button_press_type type;
		/* Compare timestamps of each time the timer handler is called
		 * (one on button press, another on button release )*/
		button_press_duration = k_uptime_delta(&last_button_press_time);

		/* Set type based on button press duration */
		if (button_press_duration <= max_button_press_timeout) {
			type = BUTTON_PRESS_NORMAL;
		} else {
			type = BUTTON_PRESS_TIMEOUT;
		}

		/* Invoke the registered user callback handler */
		registered_callback(last_button_press_time, button_press_duration, type);

		/* Reset last_button_press_time to capture next button press event */
		last_button_press_time = 0;

		button_state = BUTTON_CB_STATE_INIT;
	}
}

int button_press_module_init(uint32_t max_timeout, button_press_callback_t callback)
{
	/* Cannot initialize new paramters of button press without calling deinit first */
	if (BUTTON_CB_STATE_DEINIT < button_state) {
		return -EBUSY;
	}
	/* Invalid params */
	if (0 == max_timeout || NULL == callback) {
		return -EINVAL;
	}

	int ret = 0;

	max_button_press_timeout = max_timeout;
	registered_callback = callback;

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
	if (ret != 0) {
		printf("Error %d: failed to configure %s pin %d\n", ret, button.port->name,
		       button.pin);
		button_state = BUTTON_CB_STATE_ERROR;
		return ret;
	}

	/* Configuree interrupts on both edges so we can sample
	 * timestamp to calculate duration of button press */
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		printf("Error %d: failed to configure interrupt on %s pin %d\n", ret,
		       button.port->name, button.pin);
		button_state = BUTTON_CB_STATE_ERROR;
		return ret;
	}
	gpio_init_callback(&button_cb_data, button_press_handler, BIT(button.pin));
	gpio_add_callback_dt(&button, &button_cb_data);
	printf("Set up button at %s pin %d\n\n", button.port->name, button.pin);

	k_timer_init(&debounce_timer, button_debounce_timer_handler, NULL);

	/* Set state of button as configured. Only one configruation can exist at a time */
	button_state = BUTTON_CB_STATE_INIT;

	return ret;
}

int button_press_module_deinit(void)
{
	if (BUTTON_CB_STATE_DEINIT == button_state) {
		return 0;
	}

	int ret = 0;
	max_button_press_timeout = 0;
	registered_callback = NULL;

	/* Disconnect the GPIO from the MCU */
	ret = gpio_pin_configure_dt(&button, GPIO_DISCONNECTED);
	if (ret != 0) {
		printf("Error %d: failed to disconnect %s pin %d\n", ret, button.port->name,
		       button.pin);
		button_state = BUTTON_CB_STATE_ERROR;
		return ret;
	}

	/* Disable all interrupts on it */
	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_DISABLE);
	if (ret != 0) {
		printf("Error %d: failed to disable interrupt on %s pin %d\n", ret,
		       button.port->name, button.pin);
		button_state = BUTTON_CB_STATE_ERROR;
		return ret;
	}

	gpio_remove_callback_dt(&button, &button_cb_data);
	if (ret != 0) {
		printf("Error %d: failed to remove intr callback on %s pin %d\n", ret,
		       button.port->name, button.pin);
		button_state = BUTTON_CB_STATE_ERROR;
		return ret;
	}

	k_timer_stop(&debounce_timer);

	/* Set the button state to undefined so a new configruation 
	 * can be set by calling the init function */
	button_state = BUTTON_CB_STATE_DEINIT;

	return ret;
}

button_cb_state get_button_cb_state(void)
{
	/* Return current state of button */
	return button_state;
}
