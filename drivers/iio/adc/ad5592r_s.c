// SPDX-License-Identifier: GPL-2.0
/*
 * ADI ad5592r_s ADC driver
 *
 * Copyright 2016 Analog Devices Inc.
 */

#include <linux/module.h>

#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

struct adi_ad5592r_state {
	bool en;
	u16 tmp_chan[6];
};


static int adi_ad5592r_read_raw(struct iio_dev *indio_dev,
	 	 	    struct iio_chan_spec const *chan,
			    int *val,
			    int *val2,
			    long mask)
{

struct adi_ad5592r_state *st = iio_priv(indio_dev);

switch (mask)
{
case IIO_CHAN_INFO_RAW:
	switch(chan->channel)
		{
		case 0: *val = st->tmp_chan[0]; break;
	  	case 1: *val = st->tmp_chan[1]; break;
		case 2: *val = st->tmp_chan[2]; break;
		case 3: *val = st->tmp_chan[3]; break;
		case 4: *val = st->tmp_chan[4]; break;
		case 5: *val = st->tmp_chan[5]; break;
		}

	return IIO_VAL_INT;

case IIO_CHAN_INFO_ENABLE:
		*val = st->en;
		return IIO_VAL_INT;
default:
	return -EINVAL;
}

}

static int adi_ad5592r_write_raw(struct iio_dev *indio_dev,
	 	 	    struct iio_chan_spec const *chan,
			    int val,
			    int val2,
			    long mask)
{
	struct adi_ad5592r_state *st = iio_priv(indio_dev);

	switch (mask)
{
	case IIO_CHAN_INFO_RAW:
		switch(chan->channel)
		{
		case 0: st->tmp_chan[0] = val; break;
	  	case 1: st->tmp_chan[1] = val; break;
		case 2: st->tmp_chan[2] = val; break;
		case 3: st->tmp_chan[3] = val; break;
		case 4: st->tmp_chan[4] = val; break;
		case 5: st->tmp_chan[5] = val; break;
		}
		return 0;

	case IIO_CHAN_INFO_ENABLE:
		st->en = val;
		return 0;
	default:
		return -EINVAL;
}
}

static const struct iio_info ad5592r_info = {
	.read_raw = &adi_ad5592r_read_raw,
	.write_raw = &adi_ad5592r_write_raw,
};

static const struct iio_chan_spec adi_ad5592r_channel[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1, //seteaza canalul, e ca un true
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},

};

static int ad5592r_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct adi_ad5592r_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if (!indio_dev)
		return -ENOMEM;
	
	st = iio_priv(indio_dev); 
	st->en = 0;
	st->tmp_chan[6] = 0;

	indio_dev -> name = "iio-ad5592r";
	indio_dev -> info = &ad5592r_info;
	indio_dev -> channels = adi_ad5592r_channel;
	indio_dev -> num_channels = ARRAY_SIZE(adi_ad5592r_channel);

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