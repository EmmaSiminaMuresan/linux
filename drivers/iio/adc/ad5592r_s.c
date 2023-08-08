// SPDX-License-Identifier: GPL-2.0
/*
 * ADI ad5592r_s ADC driver
 *
 * Copyright 2016 Analog Devices Inc.
 */

#include <linux/module.h>

#include <linux/spi/spi.h>

#include <linux/iio/iio.h>



static const struct iio_info ad5592r_info = {

};

static int ad5592r_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if (!indio_dev)
		return -ENOMEM;

	indio_dev -> name = "iio-ad5592r";
	indio_dev -> info = &ad5592r_info;

	return devm_iio_device_register(&spi->dev, indio_dev);

}

static struct spi_driver ad5592r_driver = {
	.driver = {
		.name = "ad5592rs",
	},
	.probe = ad5592r_probe,
};
module_spi_driver(ad5592r_driver);

 MODULE_AUTHOR("Thomas Bontya <thomasbontya@gmail.com>");
 MODULE_DESCRIPTION("Analog Devices ad5592r ADC Driver");
 MODULE_LICENSE("GPL v2");