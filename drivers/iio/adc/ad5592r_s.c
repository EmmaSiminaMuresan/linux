// SPDX-License-Identifier: GPL-2.0-only
/*
 * AD5592R Digital <-> Analog converters driver
 * Link: https://wiki.analog.com/resources/fpga/docs/axi_adc_ip
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <asm/unaligned.h>
#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>

#define AD5592R_S_REG_READBACK 0x7
#define AD5592R_S_MASK_RDB_EN  BIT(6)
#define AD5592R_S_MASK_RDB_REG GENMASK(5, 2)

#define AD5592R_S_ADDR_MASK    GENMASK(14, 11)
#define AD5592R_S_VAL_MASK     GENMASK(10, 0)

struct ad5592r_s_state {
	struct spi_device *spi;
};

static int ad5592r_s_spi_write_ctl(struct ad5592r_s_state *st, u8 reg, u16 val)
{
	u16 msg = 0;
	u16 tx = 0;

	struct spi_transfer xfer = {
		.tx_buf = &tx,
		.len = 2,
	};

	msg |= FIELD_PREP(AD5592R_S_ADDR_MASK, reg);
	msg |= FIELD_PREP(AD5592R_S_VAL_MASK, val);

	put_unaligned_be16(msg, &tx);

	return spi_write(st->spi, &xfer, sizeof(tx));
}

static int ad5592r_s_nop(struct ad5592r_s_state *st, u16 *val)
{
	struct spi_transfer xfer = {
		.tx_buf = 0,
		.rx_buf = val,
		.len = 2,
	};

	return spi_sync_transfer(st->spi, &xfer, 1);
}

static int ad5592r_s_spi_read_ctl(struct ad5592r_s_state *st, u8 reg, u16 *val)
{
	u16 msg = 0;
	u16 tx = 0;
	u16 rx = 0;
	int ret;

	struct spi_transfer xfer = {
		.tx_buf = &tx,
		.len = 2,
	};

	msg |= FIELD_PREP(AD5592R_S_ADDR_MASK, AD5592R_S_REG_READBACK);
	msg |= AD5592R_S_MASK_RDB_EN;
	msg |= FIELD_PREP(AD5592R_S_MASK_RDB_REG, reg);

	put_unaligned_be16(msg, &tx);

	ret = spi_sync_transfer(st->spi, &xfer, 1);
	if(ret)
	{
		dev_err(&st->spi->dev, "Failed at SPI WR transfer");
		return ret;
	}

	ret = ad5592r_s_nop(st, &rx);
	if(ret)
	{
		dev_err(&st->spi->dev, "Failed at SPI WR NOP transfer");
		return ret;
	}

	*val = get_unaligned_be16(&rx);

	return 0;
}

static int ad5592r_s_read_raw(struct iio_dev *indio_dev,
			      struct iio_chan_spec const *chan,
			      int *val,
			      int *val2,
			      long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_ENABLE:
		*val = 0;
		return IIO_VAL_INT;
	case IIO_CHAN_INFO_RAW:
		if (chan->channel)
			*val = 0;
		else
			*val = 0;
		return IIO_VAL_INT;
	}
	return -EINVAL;
}


static int ad5592r_s_write_raw(struct iio_dev *indio_dev,
			       struct iio_chan_spec const *chan,
			       int val,
			       int val2,
			       long mask)
{
	switch (mask) {
	case IIO_CHAN_INFO_ENABLE:
		return 0;
	}

	return -EINVAL;
}

static int ad5592r_s_reg_access(struct iio_dev *indio_dev,
			        unsigned reg, unsigned writeval,
			        unsigned *readval)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	if (readval)
		return ad5592r_s_spi_read_ctl(st, reg, (u16 *)readval);

	return 	ad5592r_s_spi_write_ctl(st, reg, writeval);
}

static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
	.debugfs_reg_access = &ad5592r_s_reg_access,
};

static const struct iio_chan_spec ad5592r_s_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate =BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate =BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate =BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate =BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate =BIT(IIO_CHAN_INFO_RAW),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate =BIT(IIO_CHAN_INFO_RAW),
	}
};

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ad5592r_s_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	if(!indio_dev)
	{
		return -ENOMEM;
	}

	st = iio_priv(indio_dev);
	indio_dev->name = "ad5592r_s";
	indio_dev->channels = ad5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channels);
	indio_dev->info = &ad5592r_s_info;

	st->spi = spi;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
	.driver = {
		.name = "ad5592r_s",
	},
	.probe = ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);


MODULE_AUTHOR("Alexandru-Teodor Bocioanca <alexbocioanca123@gmail.com>");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");