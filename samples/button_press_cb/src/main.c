/*
 * Copyright (c) 2023 T-Mobile USA, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <inttypes.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "button.h"

#define FACTORY_RESET_TIMEOUT_MS (10 * 1000)

static k_tid_t main_thread_id = 0;

/*
 * User callbacks to be called when button is pressed
 */

static void factory_reset_callback(uint32_t timestamp, uint32_t duration, button_press_type type)
{
	printf("%s: Button pressed: timestamp: %u; duration: %u, button press type: %s\n",
			__func__, timestamp, duration,
	       type == BUTTON_PRESS_TIMEOUT ? "BUTTON_PRESS_TIMEOUT" : "BUTTON_PRESS_NORMAL");

	/* Wakeup the main thread to proceed with other reinint */
	k_wakeup(main_thread_id);
}

static void do_something_else_on_button_press(uint32_t timestamp, uint32_t duration,
					      button_press_type type)
{
	printf("%s: Button pressed: timestamp: %u; duration: %u, button press type: %s\n", __func__,
	       timestamp, duration,
	       type == BUTTON_PRESS_TIMEOUT ? "BUTTON_PRESS_TIMEOUT" : "BUTTON_PRESS_NORMAL");
}

/*
 *	Print greeting
 */
static void greeting()
{
	puts("\n\n\t\t\tWelcome to T-Mobile DevEdge!\n\n"
	     "This application aims to demonstrate the button press callback sample module which\n"
	     "uses SW0 interrupt pin, which is connected to the user pushbutton switch of the\n"
	     "DevEdge module.\n");
}

/* Entry point */
void main(void)
{
	int ret = 0;
	greeting();

	main_thread_id = k_current_get();

	/* Initialize the button with the callback and max timeout */
	ret = button_press_module_init(FACTORY_RESET_TIMEOUT_MS, factory_reset_callback);
	if (ret != 0) {
		printf("Error %d: failed to configure button press module\n", ret);
	}

	/* Wait until a callback occurs atleast once */
	k_sleep(K_FOREVER);

	/* If previously configured, deinit the button callback */
	if (get_button_cb_state() == BUTTON_CB_STATE_INIT) {
		ret = button_press_module_deinit();
		if (ret != 0) {
			printf("Error %d: failed to deinit button callback module\n", ret);
		}
	}

	/* Initialize the button callback with new parameters after successful deinit */
	if (get_button_cb_state() == BUTTON_CB_STATE_DEINIT) {
		ret = button_press_module_init((5 * 1000), do_something_else_on_button_press);
		if (ret != 0) {
			printf("Error %d: failed to configure button press module\n", ret);
		}
	}
}
