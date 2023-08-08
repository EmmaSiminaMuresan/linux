// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ADI AD5592R ADC driver
 *
 * Copyright (C) 2023 Freescale Semiconductor, Inc.
 */

#include <linux/module.h>
#include <linux/iio/iio.h>

#include <linux/spi/spi.h>

static const struct iio_info ad5592r_s_info ={

};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	indio_dev=devm_iio_device_alloc(&spi->dev,0);
	if(!indio_dev)
		return -ENOMEM;
	
	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;

	return devm_iio_device_register(&spi->dev,indio_dev);
}

static struct spi_driver ad5592r_s_driver=
{
    .driver = {
        .name = "ad5592r_s",
    },
    .probe = ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Copacel Narcis-MIhail<copacelnarcismihail@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD5592R ADC Driver");
MODULE_LICENSE("GPL v2");