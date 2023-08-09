// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI ADC emulator driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

struct adi_emu_state {
	bool en;
};

static int adi_emu_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val,
			    int *val2,
			    long mask)
{
	struct adi_emu_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		if (chan->channel)
			*val = 10;
		else
			*val = 22;
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_ENABLE:
		*val = st->en;
		return IIO_VAL_INT;
	default:
		return -EINVAL; //error - Invalid Value
	}
}

static int adi_emu_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val,
			     int val2,
			     long mask)
{
	struct adi_emu_state *st = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_ENABLE:
		st->en = val;
		return 0;	
	default:
		return -EINVAL; //error - Invalid Value
	}

}

static const struct iio_info adi_emu_info = {
	.read_raw = &adi_emu_read_raw,
	.write_raw = &adi_emu_write_raw,
};

static const struct iio_chan_spec adi_emu_channel[] = {
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
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
};

static int adi_emu_probe(struct spi_device *spi) {
	struct iio_dev *indio_dev;
	struct adi_emu_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (!indio_dev)
		return -ENOMEM; //eroare: Out Of Memory

	st = iio_priv(indio_dev);
	st->en = 0;

	indio_dev->name = "iio-adi-emu";
	indio_dev->info = &adi_emu_info;
	indio_dev->channels = adi_emu_channel;
	indio_dev->num_channels = ARRAY_SIZE(adi_emu_channel);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver adi_emu_driver = {
	.driver = {
		.name = "iio-adi-emu",
	},
	.probe = adi_emu_probe,
};
module_spi_driver(adi_emu_driver);

MODULE_AUTHOR("Dominic-Stefan Stavri <stavridominic@yahoo.com");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");