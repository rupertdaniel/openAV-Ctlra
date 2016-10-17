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

#ifndef OPENAV_CTLR_H
#define OPENAV_CTLR_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include <stdint.h>

/** @file
 * Ctlr is a lightweight C library for accessing controllers, to
 * make it easier to implement support for them in software. This library
 * abstracts the hardware and transport layer, providing events for all
 * inputs available, and functions that can be called to display feedback
 * on the device.
 */

#include "event.h"
#include "devices.h"

/** Callback function that is called for each event */
typedef void (*ctlr_event_func)(struct ctlr_dev_t* dev,
				  const struct ctlr_event_t* event);

/** Connect to a controller device. */
struct ctlr_dev_t *ctlr_dev_connect(enum ctlr_dev_id_t dev_id, void *future);

/** Poll device for events, causing event callback to be called for each
 * event that has occured since the last poll.
 * @param dev The Device to poll.
 * @retval  0 No events available from device.
 */
uint32_t ctlr_dev_poll(struct ctlr_dev_t *dev);

/** Disconnect from controller device, resetting to a neutral state.
 * @param dev The device to be disconnected
 * @retval 0 Successfully disconnected
 * @retval -ENOTSUP Disconnect not supported by driver
 * @retval <0 Error in disconnecting device
 */
int32_t ctlr_dev_disconnect(struct ctlr_dev_t *dev);

/** Connect function to instantiate a dev from the driver */
typedef struct ctlr_dev_t *(*ctlr_dev_connect_func)(void *future);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OPENAV_CTLR_H */
