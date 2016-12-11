/*
 * Copyright (c) 2016, OpenAV Productions,
 * Harry van Haaren <harryhaaren@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "ni_kontrol_x1_mk2.h"
#include "../device_impl.h"

#define NI_VENDOR           0x17cc
#define NI_KONTROL_X1_MK2   0x1220
#define USB_INTERFACE_ID   (0x00)
#define USB_ENDPOINT_READ  (0x81)
#define USB_ENDPOINT_WRITE (0x01)

/* This struct is a generic struct to identify hw controls */
struct ni_kontrol_x1_mk2_ctlr_t {
	int event_id;
	int buf_byte_offset;
	uint32_t mask;
};

static const char *ni_kontrol_x1_mk2_control_names[] = {
	/* Faders / Dials */
	"FX 1 Knob (Left)",
	"FX 2 Knob (Left)",
	"FX 3 Knob (Left)",
	"FX 4 Knob (Left)",
	"FX 1 Knob (Right)",
	"FX 2 Knob (Right)",
	"FX 3 Knob (Right)",
	"FX 4 Knob (Right)",
	/* Buttons */
	"Headphones Cue A",
	"Headphones Cue B",
	"Mode",
	"Filter On (Left)",
	"Filter On (Right)",
};
#define CONTROL_NAMES_SIZE (sizeof(ni_kontrol_x1_mk2_control_names) /\
			    sizeof(ni_kontrol_x1_mk2_control_names[0]))

static const struct ni_kontrol_x1_mk2_ctlr_t sliders[] = {
	{NI_KONTROL_X1_MK2_SLIDER_LEFT_FX_1    ,  1, UINT32_MAX},
	{NI_KONTROL_X1_MK2_SLIDER_LEFT_FX_2    ,  3, UINT32_MAX},
	{NI_KONTROL_X1_MK2_SLIDER_LEFT_FX_3    ,  5, UINT32_MAX},
	{NI_KONTROL_X1_MK2_SLIDER_LEFT_FX_4    ,  7, UINT32_MAX},
	{NI_KONTROL_X1_MK2_SLIDER_RIGHT_FX_1   ,  9, UINT32_MAX},
	{NI_KONTROL_X1_MK2_SLIDER_RIGHT_FX_2   , 11, UINT32_MAX},
	{NI_KONTROL_X1_MK2_SLIDER_RIGHT_FX_3   , 13, UINT32_MAX},
	{NI_KONTROL_X1_MK2_SLIDER_RIGHT_FX_4   , 15, UINT32_MAX},
};
#define SLIDERS_SIZE (sizeof(sliders) / sizeof(sliders[0]))

static const struct ni_kontrol_x1_mk2_ctlr_t encoders[] = {
	{NI_KONTROL_X1_MK2_BTN_ENCODER_MID_ROTATE  , 29, 0x10},
};
#define ENCODERS_SIZE (sizeof(encoders) / sizeof(encoders[0]))

static const struct ni_kontrol_x1_mk2_ctlr_t buttons[] = {
	/* Top left buttons */
	{NI_KONTROL_X1_MK2_BTN_LEFT_FX_1 , 19, 0x80},
	{NI_KONTROL_X1_MK2_BTN_LEFT_FX_2 , 19, 0x40},
	{NI_KONTROL_X1_MK2_BTN_LEFT_FX_3 , 19, 0x20},
	{NI_KONTROL_X1_MK2_BTN_LEFT_FX_4 , 19, 0x10},
	/* Top right buttons */
	{NI_KONTROL_X1_MK2_BTN_RIGHT_FX_1, 19, 0x08},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_FX_2, 19, 0x04},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_FX_3, 19, 0x02},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_FX_4, 19, 0x01},
	/* Smaller square FX buttons in screen area */
	{NI_KONTROL_X1_MK2_BTN_LEFT_FX_SELECT1 , 20, 0x80},
	{NI_KONTROL_X1_MK2_BTN_LEFT_FX_SELECT2 , 20, 0x40},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_FX_SELECT1, 20, 0x20},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_FX_SELECT2, 20, 0x10},
	/* Arrow / Shift buttons between encoders */
	{NI_KONTROL_X1_MK2_BTN_LEFT_ARROW      , 20, 0x08},
	{NI_KONTROL_X1_MK2_BTN_SHIFT           , 20, 0x04},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_ARROW     , 20, 0x02},
	{NI_KONTROL_X1_MK2_BTN_ENCODER_RIGHT   , 20, 0x01},
	/* Right lower btn controls */
	{NI_KONTROL_X1_MK2_BTN_RIGHT_1   , 21, 0x80},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_2   , 21, 0x40},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_3   , 21, 0x20},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_4   , 21, 0x10},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_FLUX, 21, 0x08},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_SYNC, 21, 0x04},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_CUE , 21, 0x02},
	{NI_KONTROL_X1_MK2_BTN_RIGHT_PLAY, 21, 0x01},
	/* Left lower btn controls */
	{NI_KONTROL_X1_MK2_BTN_LEFT_1   , 22, 0x80},
	{NI_KONTROL_X1_MK2_BTN_LEFT_2   , 22, 0x40},
	{NI_KONTROL_X1_MK2_BTN_LEFT_3   , 22, 0x20},
	{NI_KONTROL_X1_MK2_BTN_LEFT_4   , 22, 0x10},
	{NI_KONTROL_X1_MK2_BTN_LEFT_FLUX, 22, 0x08},
	{NI_KONTROL_X1_MK2_BTN_LEFT_SYNC, 22, 0x04},
	{NI_KONTROL_X1_MK2_BTN_LEFT_CUE , 22, 0x02},
	{NI_KONTROL_X1_MK2_BTN_LEFT_PLAY, 22, 0x01},
	/* Encoder movement and touch */
	{NI_KONTROL_X1_MK2_BTN_ENCODER_RIGHT_TOUCH, 23, 0x10},
	{NI_KONTROL_X1_MK2_BTN_ENCODER_MID_TOUCH  , 23, 0x08},
	{NI_KONTROL_X1_MK2_BTN_ENCODER_LEFT_TOUCH , 23, 0x04},
	{NI_KONTROL_X1_MK2_BTN_ENCODER_MID_PRESS  , 23, 0x02},
	{NI_KONTROL_X1_MK2_BTN_ENCODER_LEFT_PRESS , 23, 0x01},
};
#define BUTTONS_SIZE (sizeof(buttons) / sizeof(buttons[0]))

#define CONTROLS_SIZE (SLIDERS_SIZE + BUTTONS_SIZE)

struct ni_kontrol_x1_mk2_controls_t {
	uint8_t waste;
	/* 1 */
	uint16_t left_fx_1;
	uint16_t left_fx_2;
	uint16_t left_fx_3;
	uint16_t left_fx_4;
	uint16_t right_fx_1;
	uint16_t right_fx_2;
	uint16_t right_fx_3;
	uint16_t right_fx_4;

	/* 17 */
	uint8_t encoder_middle : 4; // & 0xF0;
	uint8_t encoder_left   : 4; // & 0x0F;
	uint8_t waste2;
	uint8_t encoder_right  : 4; // & 0xF;

	/* 19 */
	uint8_t btn_left_fx_1  : 1; // 0x80
	uint8_t btn_left_fx_2  : 1; // 0x40
	uint8_t btn_left_fx_3  : 1; // 0x20
	uint8_t btn_left_fx_4  : 1; // 0x10
	uint8_t btn_right_fx_1 : 1; // 0x08
	uint8_t btn_right_fx_2 : 1; // 0x04
	uint8_t btn_right_fx_3 : 1; // 0x02
	uint8_t btn_right_fx_4 : 1; // 0x01

	/* 20 */
	uint8_t btn_fx_left_select1  : 1; // 0x80
	uint8_t btn_fx_left_select2  : 1; // 0x40
	uint8_t btn_fx_right_select1 : 1; // 0x20
	uint8_t btn_fx_right_select2 : 1; // 0x10
	uint8_t btn_left_arrow       : 1; // 0x08
	uint8_t btn_shift            : 1; // 0x04
	uint8_t btn_right_arrow      : 1; // 0x02
	uint8_t encoder_button_right : 1; // 0x01

	/* 21 */
	uint8_t right_1    : 1; // 0x80
	uint8_t right_2    : 1; // 0x40
	uint8_t right_3    : 1; // 0x20
	uint8_t right_4    : 1; // 0x10
	uint8_t right_flux : 1; // 0x08
	uint8_t right_sync : 1; // 0x04
	uint8_t right_cue  : 1; // 0x02
	uint8_t right_play : 1; // 0x01

	/* 22 */
	uint8_t left_1    : 1; // 0x80
	uint8_t left_2    : 1; // 0x40
	uint8_t left_3    : 1; // 0x20
	uint8_t left_4    : 1; // 0x10
	uint8_t left_flux : 1; // 0x08
	uint8_t left_sync : 1; // 0x04
	uint8_t left_cue  : 1; // 0x02
	uint8_t left_play : 1; // 0x01

	/* 23 */
	uint8_t encoder_right_touch  : 1; // 0x10
	uint8_t encoder_middle_touch : 1; // 0x08
	uint8_t encoder_left_touch   : 1; // 0x04
	uint8_t encoder_middle_press : 1; // 0x02
	uint8_t encoder_left_press   : 1; // 0x01

	/* 24 */
	uint8_t touch_strip_lsb;
	uint8_t touch_strip_msb;
};
#warning TODO TouchStrip

struct ni_kontrol_x1_mk2_t {
	/* base handles usb i/o etc */
	struct ctlr_dev_t base;

	/* current value of each controller is stored here */
	float hw_values[CONTROLS_SIZE];

	/* current state of the lights, only flush on dirty */
	uint8_t lights_dirty;
	uint8_t lights[NI_KONTROL_X1_MK2_LED_COUNT];
};

static uint32_t ni_kontrol_x1_mk2_poll(struct ctlr_dev_t *dev);
static int32_t ni_kontrol_x1_mk2_disconnect(struct ctlr_dev_t *dev);
static int32_t ni_kontrol_x1_mk2_disconnect(struct ctlr_dev_t *dev);
static void ni_kontrol_x1_mk2_light_set(struct ctlr_dev_t *dev,
				    uint32_t light_id,
				    uint32_t light_status);
static void ni_kontrol_x1_mk2_light_flush(struct ctlr_dev_t *base,
				      uint32_t force);

static const char *
ni_kontrol_x1_mk2_control_get_name(struct ctlr_dev_t *base,
			       uint32_t control_id)
{
	struct ni_kontrol_x1_mk2_t *dev = (struct ni_kontrol_x1_mk2_t *)base;
	if(control_id < CONTROL_NAMES_SIZE)
		return ni_kontrol_x1_mk2_control_names[control_id];
	return 0;
}

struct ctlr_dev_t *ni_kontrol_x1_mk2_connect(ctlr_event_func event_func,
				  void *userdata, void *future)
{
	(void)future;
	struct ni_kontrol_x1_mk2_t *dev = calloc(1, sizeof(struct ni_kontrol_x1_mk2_t));
	if(!dev)
		goto fail;

	int err = ctlr_dev_impl_usb_open((struct ctlr_dev_t *)dev,
					 NI_VENDOR, NI_KONTROL_X1_MK2,
					 USB_INTERFACE_ID, 0);
	if(err) {
		printf("error conencting to Kontrol X1 MK2 controller, is it plugged in?\n");
		return 0;
	}

	dev->base.poll = ni_kontrol_x1_mk2_poll;
	dev->base.disconnect = ni_kontrol_x1_mk2_disconnect;
	dev->base.light_set = ni_kontrol_x1_mk2_light_set;
	dev->base.control_get_name = ni_kontrol_x1_mk2_control_get_name;
	dev->base.light_flush = ni_kontrol_x1_mk2_light_flush;

	dev->base.event_func = event_func;
	dev->base.event_func_userdata = userdata;

	return (struct ctlr_dev_t *)dev;
fail:
	free(dev);
	return 0;
}

static uint32_t ni_kontrol_x1_mk2_poll(struct ctlr_dev_t *base)
{
	struct ni_kontrol_x1_mk2_t *dev = (struct ni_kontrol_x1_mk2_t *)base;
	uint8_t buf[1024];
	int32_t nbytes;

	do {
		int handle_idx = 0;
		nbytes = ctlr_dev_impl_usb_xfer(base, handle_idx,
							 USB_ENDPOINT_READ,
							 buf, 1024);
		if(nbytes == 0)
			return 0;

#define GREEN   "\x1b[32m"
#define RESET   "\x1b[0m"

		for(int i = 0; i < 31; i++) {
			char v = buf[i];
			if(v)
				printf("%s%02x%s ", GREEN, v, RESET);
			else
				printf("%02x ", v);
		}
		printf("\n");

		struct ni_kontrol_x1_mk2_controls_t *c = (void *)buf;

		switch(nbytes) {
		case 31: {
			for(uint32_t i = 0; i < SLIDERS_SIZE; i++) {
				int id     = sliders[i].event_id;
				int offset = sliders[i].buf_byte_offset;
				int mask   = sliders[i].mask;

				uint16_t v = *((uint16_t *)&buf[offset]) & mask;
				if(dev->hw_values[i] != v) {
					dev->hw_values[i] = v;
					struct ctlr_event_t event = {
						.id = CTLR_EVENT_SLIDER,
						.slider  = {
							.id = id,
							.value = v / 4096.f},
					};
					struct ctlr_event_t *e = {&event};
					dev->base.event_func(&dev->base, 1, &e,
							     dev->base.event_func_userdata);
				}
			}
			for(uint32_t i = 0; i < BUTTONS_SIZE; i++) {
				int id     = buttons[i].event_id;
				int offset = buttons[i].buf_byte_offset;
				int mask   = buttons[i].mask;

				uint16_t v = *((uint16_t *)&buf[offset]) & mask;
				int value_idx = SLIDERS_SIZE + i;

				if(dev->hw_values[value_idx] != v) {
					dev->hw_values[value_idx] = v;

					struct ctlr_event_t event = {
						.id = CTLR_EVENT_BUTTON,
						.button  = {
							.id = id,
							.pressed = v > 0},
					};
					struct ctlr_event_t *e = {&event};
					dev->base.event_func(&dev->base, 1, &e,
							     dev->base.event_func_userdata);
				}
			}
			break;
			}
		}
	} while (nbytes > 0);


	return 0;
}

static int32_t ni_kontrol_x1_mk2_disconnect(struct ctlr_dev_t *base)
{
	struct ni_kontrol_x1_mk2_t *dev = (struct ni_kontrol_x1_mk2_t *)base;

	/* Turn off all lights */
	memset(&dev->lights[1], 0, NI_KONTROL_X1_MK2_LED_COUNT);
	dev->lights[0] = 0x80;
	ni_kontrol_x1_mk2_light_set(base, 0, 0);
	ni_kontrol_x1_mk2_light_flush(base, 0);

	free(dev);
	return 0;
}

static void ni_kontrol_x1_mk2_light_set(struct ctlr_dev_t *base,
				    uint32_t light_id,
				    uint32_t light_status)
{
	struct ni_kontrol_x1_mk2_t *dev = (struct ni_kontrol_x1_mk2_t *)base;
	int ret;

	if(!dev || light_id > NI_KONTROL_X1_MK2_LED_COUNT)
		return;

	dev->lights_dirty = 1;

	uint32_t blink  = (light_status >> 31);
	uint32_t bright = (light_status >> 24) & 0x7F;
	uint32_t r      = (light_status >> 16) & 0xFF;
	uint32_t g      = (light_status >>  8) & 0xFF;
	uint32_t b      = (light_status >>  0) & 0xFF;

#ifdef DEBUG_PRINTS
	printf("%s : dev %p, light %d, status %d\n", __func__, dev,
	       light_id, light_status);
	printf("decoded: blink[%d], bright[%d], r[%d], g[%d], b[%d]\n",
	       blink, bright, r, g, b);
#endif

	/* write brighness to all LEDs */
	dev->lights[light_id] = bright;
	dev->lights[0] = 0x80;

	/* FX ON buttons have orange and blue */
	if(light_id == NI_KONTROL_X1_MK2_LED_FX_ON_LEFT ||
	   light_id == NI_KONTROL_X1_MK2_LED_FX_ON_RIGHT) {
		dev->lights[light_id  ] = r;
		dev->lights[light_id+1] = b;
	}

	return;
}

void ni_kontrol_x1_mk2_light_flush(struct ctlr_dev_t *base, uint32_t force)
{
	struct ni_kontrol_x1_mk2_t *dev = (struct ni_kontrol_x1_mk2_t *)base;
	if(!dev->lights_dirty && !force)
		return;

	int ret = ctlr_dev_impl_usb_xfer(base,
					 USB_INTERFACE_ID,
					 USB_ENDPOINT_WRITE,
					 dev->lights,
					 NI_KONTROL_X1_MK2_LED_COUNT);
	if(ret < 0)
		printf("%s write failed!\n", __func__);
}
