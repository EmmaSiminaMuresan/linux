// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for AD5592R ADC
 *
 * Copyright (c) 2023 Analog Devices
 */

#include <linux/module.h>
#include <linux/iio/iio.h>
#include <linux/spi/spi.h>

struct adi_ad5592r_s_state {
	bool en;
	u16 temp_chann [6];
};

static int adi_ad5592r_s_read_raw(struct iio_dev *indio_dev,
			    	  struct iio_chan_spec const *chan,
			    	  int *val,
			    	  int *val2,
			    	  long mask)
{
	struct adi_ad5592r_s_state *st = iio_priv(indio_dev);  

	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:
		switch (chan->channel)
		{
		case 0:
			*val = st->temp_chann[0];
			break;
		case 1:
			*val = st->temp_chann[1];
			break;
		case 2:
			*val = st->temp_chann[2];
			break;
		case 3:
			*val = st->temp_chann[3];
			break;
		case 4:
			*val = st->temp_chann[4];
			break;
		case 5:
			*val = st->temp_chann[5];
			break;																
		default:
			return -EINVAL;
		}
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->en;
		return IIO_VAL_INT; 
	default: // invalid channel
		return -EINVAL;
	}
}

static int adi_ad5592r_s_write_raw(struct iio_dev *indio_dev,
			    	   struct iio_chan_spec const *chan,
			    	   int val,
			    	   int val2,
			    	   long mask)
{
	struct adi_ad5592r_s_state *st = iio_priv(indio_dev);  
	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:
		switch (chan->channel)
		{
		case 0:
			st->temp_chann[0]= val;
			break;
		case 1:
			st->temp_chann[1]= val;
			break;
		case 2:
			st->temp_chann[2]= val;
			break;
		case 3:
			st->temp_chann[3]= val;
			break;
		case 4:
			st->temp_chann[4]= val;
			break;
		case 5:
			st->temp_chann[5]= val;
			break;																
		default:
			return -EINVAL;
		}
		return 0;
	case IIO_CHAN_INFO_ENABLE:
		st->en = val;
		return 0; 
	default: // invalid channel
		return -EINVAL;
	}
}

static const struct iio_info adi_ad5592r_s_info = {
	.read_raw =  &adi_ad5592r_s_read_raw,
	.write_raw = &adi_ad5592r_s_write_raw,
};

static const struct iio_chan_spec adi_5592r_s_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
};

static int adi_ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct adi_ad5592r_s_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if(!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);
	st->en = 0;
	st->temp_chann[0] = 0;
	st->temp_chann[1] = 0;
	st->temp_chann[2] = 0;
	st->temp_chann[3] = 0;
	st->temp_chann[4] = 0;
	st->temp_chann[5] = 0;
	// for(u16 i = 0; i<6; i++)
	// 	st->temp_chann[i] = 0;

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &adi_ad5592r_s_info;
	indio_dev->channels = adi_5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(adi_5592r_s_channels);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver adi_ad5592r_s_driver = {
	.driver = {
		.name = "adi_d5592r_s",
	},
	.probe = adi_ad5592r_s_probe,
};

module_spi_driver(adi_ad5592r_s_driver); // inregistreaza modulul ca driver de SPI

MODULE_AUTHOR("Bogdan Stanea <Adrian.Stanea@analog.com>");
MODULE_DESCRIPTION("Analog Devices driver for AD5592R ADC");
MODULE_LICENSE("GPL v2");

