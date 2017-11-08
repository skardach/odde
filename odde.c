/*
 *  feeb.c - ACPI Button Driver
 *
 *  Copyright (C) 2017 Stanislaw Kardach <stanislaw.kardach@gmail.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <acpi/button.h>

#define ACPI_BUTTON_CLASS	"fujitsu"
#define ACPI_BUTTON_SUBCLASS	"odde"
#define ACPI_BUTTON_HID		"PNP0C32"
#define ACPI_BUTTON_DEVICE_NAME	"Optical Disc Drive Eject Button"

#define ACPI_BUTTON_TYPE_UNKNOWN	0x00
#define ACPI_BUTTON_NOTIFY_STATUS	0x80

ACPI_MODULE_NAME("odde");

MODULE_AUTHOR("Stanislaw Kardach");
MODULE_DESCRIPTION("Fujitsu Optical Disc Drive Eject Button Driver");
MODULE_LICENSE("GPL");

static const struct acpi_device_id odde_device_ids[] = {
	{ACPI_BUTTON_HID, 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, odde_device_ids);

static int acpi_odde_add(struct acpi_device *device);
static int acpi_odde_remove(struct acpi_device *device);
static void acpi_odde_notify(struct acpi_device *device, u32 event);

static struct acpi_driver acpi_odde_driver = {
	.name = "odde",
	.class = ACPI_BUTTON_CLASS,
	.ids = odde_device_ids,
	.ops = {
		.add = acpi_odde_add,
		.remove = acpi_odde_remove,
		.notify = acpi_odde_notify,
	},
};

struct acpi_button {
	struct input_dev *input;
	char phys[32]; /* for input device */
	unsigned long pushed;
	int last_state;
	ktime_t last_time;
	bool suspended;
};

static void acpi_odde_notify(struct acpi_device *device, u32 event)
{
	struct acpi_button *button = acpi_driver_data(device);
	struct input_dev *input;

	switch (event) {
	case ACPI_FIXED_HARDWARE_EVENT:
		event = ACPI_BUTTON_NOTIFY_STATUS;
		/* fall through */
	case ACPI_BUTTON_NOTIFY_STATUS:
		input = button->input;

		input_report_key(input, KEY_EJECTCD, 1);
		input_sync(input);
		input_report_key(input, KEY_EJECTCD, 0);
		input_sync(input);

		acpi_bus_generate_netlink_event(
				device->pnp.device_class,
				dev_name(&device->dev),
				event, ++button->pushed);
		break;
	default:
		dev_info(&device->dev, "Unsupported event [0x%x]\n", event);
		break;
	}
}

static int acpi_odde_add(struct acpi_device *device)
{
	struct acpi_button *button;
	struct input_dev *input;
	const char *hid = acpi_device_hid(device);
	char *name, *class;
	int error;

	button = kzalloc(sizeof(struct acpi_button), GFP_KERNEL);
	if (!button)
		return -ENOMEM;

	device->driver_data = button;

	button->input = input = input_allocate_device();
	if (!input) {
		error = -ENOMEM;
		goto err_free_button;
	}

	name = acpi_device_name(device);
	class = acpi_device_class(device);

	strcpy(name, ACPI_BUTTON_DEVICE_NAME);
	sprintf(class, "%s/%s",
			ACPI_BUTTON_CLASS, ACPI_BUTTON_SUBCLASS);

	snprintf(button->phys, sizeof(button->phys), "%s/button/input0", hid);

	input->name = name;
	input->phys = button->phys;
	input->id.bustype = BUS_HOST;
	input->id.product = 0x0001;
	input->dev.parent = &device->dev;

	input_set_capability(input, EV_KEY, KEY_EJECTCD);

	error = input_register_device(input);
	if (error)
		goto err_free_input;

	dev_info(&device->dev, "%s [%s]\n", name, acpi_device_bid(device));
	return 0;

err_free_input:
	input_free_device(input);
err_free_button:
	kfree(button);
	return error;
}

static int acpi_odde_remove(struct acpi_device *device)
{
	struct acpi_button *button = acpi_driver_data(device);

	input_unregister_device(button->input);
	kfree(button);
	return 0;
}

module_acpi_driver(acpi_odde_driver);
