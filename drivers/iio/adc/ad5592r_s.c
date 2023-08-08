 /*
 *
 * SPDX-License-Identifier : GPL-2.0-only
 * 
 * ADI AD5592R ADC DRIVER
 *
 * Copyright 2023 Analog Devices Inc.
 *
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>


static const struct iio_info ad5592r_s_info = {

};

static int ad5592r_s_probe(struct spi_device *spi){
	//prima functie apelata dupa ce s-a inregistrat driverul in spi
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev,0);
	if (!indio_dev)
		return -ENOMEM; //eroare de memorie -> se returneaza codul de eroare fara mesaj
	
	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;

	return devm_iio_device_register(&spi->dev,indio_dev);

};

static struct spi_driver ad5592r_s_driver = {
	.driver = {
		.name = "ad5592r_s",
	},
	.probe = ad5592r_s_probe,	
};
module_spi_driver(ad5592r_s_driver);


MODULE_AUTHOR("Emma-Simina Muresan <simina.muresan@outlook.com>");
MODULE_DESCRIPTION("Analog Devices ADI AD5592R ADC DRIVER");
MODULE_LICENSE("GPL v2");