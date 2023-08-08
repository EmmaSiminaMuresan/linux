// SPDX-License-Identifier: GPL-2.0-only
/*
 * AD5592R Zybo-Z7 driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static const struct iio_info adi5592r_zybo_info =  {
   
};

static int adi5592r_zyboz7_probe(struct spi_device *spi)
{
    struct iio_dev *indio_dev;

    indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if (!indio_dev)
		return -ENOMEM;

	indio_dev->name = "iio-ad5592r-zyboz7";
	indio_dev->info = &adi5592r_zybo_info; 
	
	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver adi5592r_zybo_driver = { 
	.driver = {
		.name = "iio-ad5592r-zyboz7",
	},
	.probe = adi5592r_zyboz7_probe,
};
module_spi_driver(adi5592r_zybo_driver);

MODULE_AUTHOR("Voic Andrei <voicandrei22@gmail.com"); 
MODULE_DESCRIPTION("Analog Devices AD5592R Zybo-Z7 Driver");
MODULE_LICENSE("GPL v2");
