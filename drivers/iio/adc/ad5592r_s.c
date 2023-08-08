// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI AD5592r ADC driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static const struct iio_info ad5592r_s_info = {

};

static int ad5592r_s_probe(struct spi_device *spi){
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, NULL);
	if(!indio_dev)
		return -ENOMEM;

	indio_dev -> name = "ad5592r_s";
	indio_dev -> info = &ad5592r_s_info;

	return devm_iio_device_register(&spi->dev,indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
	.driver  = {
		.name = "ad5592r_s",
	},
	.probe = ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);
          


MODULE_AUTHOR("Vlad Cristescu <vladcristescu1@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD5592r driver");
MODULE_LICENSE("GPL 0v2");