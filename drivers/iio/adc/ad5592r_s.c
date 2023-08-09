// SPDX-License-Identifier: GPL-2.0
/*
 * ADI ad5592r_s ADC driver
 *
 * Copyright 2016 Analog Devices Inc.
 */

#include <linux/module.h>

#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static int adi_emu_read_raw(struct iio_dev *indio_dev,
	 	 	    struct iio_chan_spec const *chan,
			    int *val,
			    int *val2,
			    long mask)
{

switch (mask)
{
case IIO_CHAN_INFO_RAW:
	switch(chan->channel)
		{
		case 0: *val = 1; break;
	  	case 1: *val = 12; break;
		case 2: *val = 13; break;
		case 3: *val = 14; break;
		case 4: *val = 15; break;
		case 5: *val = 16; break;
		}

	return IIO_VAL_INT;

default:
	return -EINVAL;
}

}

static const struct iio_info ad5592r_info = {
	.read_raw = &adi_emu_read_raw,
};

static const struct iio_chan_spec adi_emu_channel[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},

};

static int ad5592r_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;

	indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if (!indio_dev)
		return -ENOMEM;

	indio_dev -> name = "iio-ad5592r";
	indio_dev -> info = &ad5592r_info;
	indio_dev -> channels = adi_emu_channel;
	indio_dev -> num_channels = ARRAY_SIZE(adi_emu_channel);

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