// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI AD5592R ADC driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static const struct iio_info ad5592r_s_info = {

};
static int ad5592r_s_probe(struct spi_device *spi)
{
         struct iio_dev *indio_dev;

	 indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	 if (!indio_dev)
	 	return -ENOMEM; //mesaj de eroare Eroare de memorie

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;

	return devm_iio_device_register(&spi->dev,indio_dev);	 
}


static struct spi_driver adi_emu_driver = {
        .driver = {
			.name = "ad5592r_s", // similar cu nume fisier
	},
	.probe = ad5592r_s_probe,
};
module_spi_driver(adi_emu_driver);

MODULE_AUTHOR ("Pop Ioan Daniel <Pop.Ioan-daniel@analog.com>");
MODULE_DESCRIPTION("Analog Devices AD5592R Driver");
MODULE_LICENSE("GPL v2");