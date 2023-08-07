// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ADI ADC emulator driver
 *
 * Copyright (c) 2023 Analog Devices
 */

#include <linux/module.h>
#include <linux/iio/iio.h>
#include <linux/spi/spi.h>

static const struct iio_info adi_emu_info = {

};

static int adi_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, NULL);
	if(!indio_dev)
		return -ENOMEM;

	indio_dev->name = "iio-adi-emu";
	indio_dev->info = &adi_emu_info;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver adi_emu_driver = {
	.driver = {
		.name = "iio-adi-emu",
	},
	.probe = adi_emu_probe,
};

module_spi_driver(adi_emu_driver); // inregistreaza modulul ca driver de SPI

MODULE_AUTHOR("Bogdan Stanea <Adrian.Stanea@analog.com>");
MODULE_DESCRIPTION("Analog Devices ADC Emulator driver");
MODULE_LICENSE("GPL v2");

