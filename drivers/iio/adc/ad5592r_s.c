// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI ADC emulator driver
 *
 * Copyright 2022 Analog Devices Inc.
 */
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

struct ad5592r_s_state {
	bool en;
	u16 tmp_chan0;
	u16 tmp_chan1;
};


static int ad5592r_s_read_raw(struct iio_dev *indio_dev,		
			    struct iio_chan_spec const *chan,
                 	    int *val,
                	    int *val2,
        		    long mask)
{
	struct ad5592r_s_state *st= iio_priv(indio_dev);

	switch (mask) {
    	case IIO_CHAN_INFO_RAW:
		if (chan->channel)
			*val = st->tmp_chan1;
		else
			*val = st->tmp_chan0;
		return IIO_VAL_INT;

		case IIO_CHAN_INFO_ENABLE:
			*val = st->en;
			return IIO_VAL_INT;
	default:
		return -EINVAL;
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
		if (chan->channel)
			st->tmp_chan1 = val;
		else
			st->tmp_chan0 = val;
		return 0;
		case IIO_CHAN_INFO_ENABLE:
			st->en = val;
		return 0;
	default:
		return -EINVAL;
    	}
}

static struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
};
static const struct iio_chan_spec ad5592r_s_channel[] = {
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
	},
};



static int ad5592r_s_probe(struct spi_device *spi)
{

        struct iio_dev *indio_dev;
	struct ad5592r_s_state *st;

        indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
		
        if (!indio_dev)
        	return -ENOMEM;

	st = iio_priv(indio_dev);
	st->en = 0;
	st->tmp_chan0 = 0;
	st->tmp_chan1 = 0;
        indio_dev->name = "ad5592r_s";
        indio_dev->info = &ad5592r_s_info;
        indio_dev->channels = ad5592r_s_channel;
        indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channel);

        return devm_iio_device_register(&spi->dev, indio_dev);
}
static struct spi_driver ad5592r_s = {
        .driver = {
                .name = "ad5592r_s",
        },
        .probe = ad5592r_s_probe,
};  
module_spi_driver(ad5592r_s);
MODULE_AUTHOR("Alexandra Taslauan <aletaslauan@gmail.com>");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");