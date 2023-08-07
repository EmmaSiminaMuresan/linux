// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI ADC emulator driver 
 *
 * Copyright 2023 Analog Devices Inc.
 */
#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static struct iio_info adi_emu_info = {

};

static int adi_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if (!indio_dev)
		return -ENOMEM;

	indio_dev->name = "iio_adi_emu";
	indio_dev->info	= &adi_emu_info;

	return devm_iio_device_register(&spi->dev, indio_dev);	
}

static struct spi_driver adi_emu_driver = {
	.driver = {
		.name = "iio_adi_emu",
	},
	.probe = adi_emu_probe,
};
module_spi_driver(adi_emu_driver);

MODULE_AUTHOR("Beletei Nora <beleteinora4@gmail.com>");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");