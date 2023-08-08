// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI ADC AD5592R Driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static const struct iio_info ad5592r_s_info = {

};

static int ad5592r_s_probe(struct spi_device *spi) {
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, 0);

	if (!indio_dev)
		return -ENOMEM;

	indio_dev->name = "ad5592r_s";
	indio_dev->info =  &ad5592r_s_info;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
	.driver = {
		.name = "ad5592r_s",
	},
	.probe = ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Tudor Radoni <tudor.radoni@gmail.com>");
MODULE_DESCRIPTION("Analog Devices ADC AD5592R_S driver");
MODULE_LICENSE("GPL v2");
