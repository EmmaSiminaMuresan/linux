// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI ADC driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>

#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

struct ad5592r_state {
        bool en;
};

static int ad5592r_read_raw(struct iio_dev *indio_dev,
                            struct iio_chan_spec const *chan,
                            int *val,
                            int *val2,
                            long mask)
{
	struct ad5592r_state *st = iio_priv(indio_dev);
	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:
		if(chan->channel)
			*val = 10;
		else
			*val = 22;
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_ENABLE:
                *val = st->en;
                return IIO_VAL_INT;
	
	default:
		return -EINVAL;
	}
}

static int ad5592r_write_raw(struct iio_dev *indio_dev,
                            struct iio_chan_spec const *chan,
                            int val,
                            int val2,
                            long mask)
{
        struct ad5592r_state *st = iio_priv(indio_dev);
        switch (mask)
        {
        case IIO_CHAN_INFO_RAW:
                st->en = val;
	case IIO_CHAN_INFO_ENABLE:
		st->en = val;
		return 0;
        default:
                return -EINVAL;
        }
}

static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_read_raw,
	.write_raw = &ad5592r_write_raw,
};

static const struct iio_chan_spec ad5592r_channel[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
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

static int ad5592r_probe(struct spi_device *spi)
{
        struct iio_dev *indio_dev;
	struct ad5592r_state *st;
        indio_dev = devm_iio_device_alloc(&spi->dev, 0);
        if (!indio_dev)
                return -ENOMEM;

	st = iio_priv(indio_dev);
        st->en = 0;
	
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
        .probe = ad5592r_probe,
};

module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Pop Nicolae-Adrian <p.nicu173@gmail.com>");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");