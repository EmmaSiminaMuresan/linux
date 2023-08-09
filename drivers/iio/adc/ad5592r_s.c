// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI AD5592R driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

struct ad5592r_s_state {
	bool en;
	u16 tmp_chan_0;
	u16 tmp_chan_1;
	u16 tmp_chan_2;
	u16 tmp_chan_3;
	u16 tmp_chan_4;
	u16 tmp_chan_5;
};

static int ad5592r_s_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val,
			    int *val2,
			    long mask)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		switch (chan->channel)
		{
		case 0:
			*val = st->tmp_chan_0;
			break;
		case 1:
			*val = st->tmp_chan_1;
			break;
		case 2:
			*val = st->tmp_chan_2;
			break;
		case 3:
			*val = st->tmp_chan_3;
			break;
		case 4:
			*val = st->tmp_chan_4;
			break;
		case 5:
			*val = st->tmp_chan_5;
			break;
		default:
			return -EINVAL; //error - Invalid Value
		}
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->en;
		return IIO_VAL_INT;
	default:
		return -EINVAL; //error - Invalid Value
	}
}

static int ad5592r_s_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val,
			     int val2,
			     long mask)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		switch (chan->channel)
		{
		case 0:
			st->tmp_chan_0 = val;
			break;
		case 1:
			st->tmp_chan_1 = val;
			break;
		case 2:
			st->tmp_chan_2 = val;
			break;
		case 3:
			st->tmp_chan_3 = val;
			break;
		case 4:
			st->tmp_chan_4 = val;
			break;
		case 5:
			st->tmp_chan_5 = val;
			break;
		default:
			return -EINVAL; //error - Invalid Value
		}
		return 0;
	case IIO_CHAN_INFO_ENABLE:
		st->en = val;
		return 0;
	default:
		return -EINVAL; //error - Invalid Value
	}
}

static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
};

static const struct iio_chan_spec ad5592r_channel[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		// apare o singura data, daca este declarat la fiecare canal,
		// va fi ignorat
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),		
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		// apare o singura data, daca este declarat la fiecare canal,
		// va fi ignorat
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		// apare o singura data, daca este declarat la fiecare canal,
		// va fi ignorat
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		// apare o singura data, daca este declarat la fiecare canal,
		// va fi ignorat
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		// apare o singura data, daca este declarat la fiecare canal,
		// va fi ignorat
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		// apare o singura data, daca este declarat la fiecare canal,
		// va fi ignorat
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ad5592r_s_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM; //eroare: Out Of Memory
	
	st = iio_priv(indio_dev);
	st->en = 0;
	st->tmp_chan_0 = 0;
	st->tmp_chan_1 = 0;
	st->tmp_chan_2 = 0;
	st->tmp_chan_3 = 0;
	st->tmp_chan_4 = 0;
	st->tmp_chan_5 = 0;	

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = ad5592r_channel;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_channel);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
	.driver = {
		.name = "ad5592r_s",
	},
	.probe =  ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Dominic-Stefan Stavri <stavridominic@yahoo.com");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");