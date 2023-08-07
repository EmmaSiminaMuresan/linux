// SPDX-License-Identifier: GPL-2.0
/*
 * Analog Devices AD5592 SPI ADC driver
 *
 * Copyright 2022 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static int ad5592_read_raw(struct iio_dev *indio_dev,
			   struct iio_chan_spec const *chan,
			   int *val, int *val2, long info)
{
	*val = 0;
	return IIO_VAL_INT;
}

static const struct iio_info ad5592_info = {
	.read_raw = &ad5592_read_raw,
}

static int ad4630_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev; 
  
	indio_dev = devm_iio_device_alloc(&spi->dev, NULL); 
	if (!indio_dev) 
		return -ENOMEM; 
	
	indio_dev->name = "ad5592"; 
	indio_dev->info = &ad5592_info; 
	
	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592_driver = {
	.driver = {
		.name = "ad5592",
	},
	.probe = ad4630_probe,
};
module_spi_driver(ad5592_driver);


MODULE_AUTHOR("Daniel Matyas <daniel.matyas@analog.com>");
MODULE_DESCRIPTION("Analog Devices AD5592 ADC driver");
MODULE_LICENSE("GPL v2");
