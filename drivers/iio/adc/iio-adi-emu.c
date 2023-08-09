// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ADI ADC emulator driver
 *
 * Copyright (c) 2023 Analog Devices
 */

#include <linux/module.h>
#include <linux/iio/iio.h>
#include <linux/spi/spi.h>

struct adi_emu_state {
	bool en;
	u16 temp_chan0;
	u16 temp_chan1;
};

static int adi_emu_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val,
			    int *val2,
			    long mask)
{
	struct adi_emu_state *st = iio_priv(indio_dev);

	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:
		if(chan->channel) {
			*val = st->temp_chan1;
		} else {
			*val = st->temp_chan0;
		}
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->en;
		return IIO_VAL_INT;
	default:
		return -EINVAL;
	}
}

static int adi_emu_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val,
			     int val2,
			     long mask)
{
	struct adi_emu_state *st = iio_priv(indio_dev);
	
	switch(mask)
	{
	case IIO_CHAN_INFO_RAW:
		if(chan->channel) {
			st->temp_chan1 = val;
		} else {
			st->temp_chan0 = val;
		}
		return 0;

	case IIO_CHAN_INFO_ENABLE:
		st->en = val;
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct iio_info adi_emu_info = {
	.read_raw =  &adi_emu_read_raw,
	.write_raw = &adi_emu_write_raw,
};

static const struct iio_chan_spec adi_emu_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,// /sys/bus/iio canalul apare ca si channel_numar
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE), // chiar daca e adaugat si aici il vom vedea doar odata comut pt toate canalele
	}
};

static int adi_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct adi_emu_state *st;

	// allocate memory for the iio driver
	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if(!indio_dev)
		return -ENOMEM;
	
	// bind with values in the allocated space
	st = iio_priv(indio_dev);
	st->en = 0; //default start value
	st->temp_chan0 = 0;
	st->temp_chan1 = 0;

	indio_dev->name = "iio-adi-emu";
	indio_dev->info = &adi_emu_info;
	indio_dev->channels = adi_emu_channels;
	indio_dev->num_channels = ARRAY_SIZE(adi_emu_channels);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver adi_emu_driver = {
	.driver = {
		.name = "iio-adi-emu",
	},
	.probe = adi_emu_probe,
};

module_spi_driver(adi_emu_driver); // inregistreaza modulul ca driver de SPI

MODULE_AUTHOR("Bogdan Stanea <Adrian.Stanea@analog.com>");
MODULE_DESCRIPTION("Analog Devices ADC Emulator driver");
MODULE_LICENSE("GPL v2");

