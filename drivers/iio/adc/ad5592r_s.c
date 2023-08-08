// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for AD5592R ADC
 *
 * Copyright (c) 2023 Analog Devices
 */

#include <linux/module.h>
#include <linux/iio/iio.h>
#include <linux/spi/spi.h>

static const struct iio_info adi_ad5592r_s_info = {

};

static int adi_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if(!indio_dev)
		return -ENOMEM;

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &adi_ad5592r_s_info;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver adi_emu_driver = {
	.driver = {
		.name = "adi_d5592r_s",
	},
	.probe = adi_emu_probe,
};

module_spi_driver(adi_emu_driver); // inregistreaza modulul ca driver de SPI

MODULE_AUTHOR("Bogdan Stanea <Adrian.Stanea@analog.com>");
MODULE_DESCRIPTION("Analog Devices driver for AD5592R ADC");
MODULE_LICENSE("GPL v2");

