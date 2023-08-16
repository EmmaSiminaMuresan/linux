// SPDX-License-Identifier: GPL-2.0
/*
 * ADI ad5592r_s ADC driver
 *
 * Copyright 2016 Analog Devices Inc.
 */

#include <asm/unaligned.h>
#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

#define AD5592R_S_REG_READBACK	0x7
#define  AD5592R_S_MASK_RDB_EN	BIT(6)
#define  AD5592R_S_MASK_RDB_REG	GENMASK(5, 2)

#define AD5592R_S_ADDR_MASK	GENMASK(14, 11)
#define AD5592R_S_VAL_MASK	GENMASK(10, 0)

struct ad5592r_s_state
{
	struct spi_device *spi;
	bool en;
	u16 tmp_chan[6];
};

static int ad5592r_s_spi_nop(struct ad5592r_s_state *st, u16 *val)
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
	u16 tx = 0;
	u16 msg = 0;
	u16 rx = 0;
	int ret = 0;

	struct spi_transfer xfer = {
		.tx_buf = &tx,
		.len = 2,
	};

	msg  = FIELD_PREP(AD5592R_S_ADDR_MASK, AD5592R_S_REG_READBACK);
	msg |= AD5592R_S_MASK_RDB_EN;
	msg |= FIELD_PREP(AD5592R_S_MASK_RDB_REG, reg);

	put_unaligned_be16(msg, &tx);

	// make spi transfer
	ret = spi_sync_transfer(st->spi, &xfer, 1);
	if (ret) {
		dev_err(&st->spi->dev, "Failed at SPI WR transfer\n");
		return ret;
	}

	ret = ad5592r_s_spi_nop(st, &rx);
	if (ret) {
		dev_err(&st->spi->dev, "Failed at SPI WR NOP transfer\n");
		return ret;
	}
	
	*val = get_unaligned_be16(&rx);
	return 0;
}

// Write to a register
static int ad5592r_s_spi_write(struct ad5592r_s_state *st, u8 reg, u16 val)
{
	u16 msg = 0;
	u16 tx = 0;

	struct spi_transfer xfer = {
		.tx_buf = &tx,
		.len = 2, // 
	};

	// set register address and value
	msg  = FIELD_PREP(AD5592R_S_ADDR_MASK, reg);
	msg |= FIELD_PREP(AD5592R_S_VAL_MASK, val);
	// dev_info(&st->spi->dev, "msg at write: 0x%x\n", msg);

	// convert to big endian
	put_unaligned_be16(msg, &tx);
	// dev_info(&st->spi->dev, "tx at write: 0x%x\n", tx);

	return spi_sync_transfer(st->spi, &xfer, 1);
}

static int ad5592r_s_read_raw(struct iio_dev *indio_dev,
			      struct iio_chan_spec const *chan,
			      int *val,
			      int *val2,
			      long mask)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);
	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:
		switch (chan->channel)
		{
		case 0:
			*val = st->tmp_chan[0];
			break;
		case 1:
			*val = st->tmp_chan[1];
			break;
		case 2:
			*val = st->tmp_chan[2];
			break;
		case 3:
			*val = st->tmp_chan[3];
			break;
		case 4:
			*val = st->tmp_chan[4];
			break;
		case 5:
			*val = st->tmp_chan[5];
			break;
		default:
			return -EINVAL;
			break;
		}
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
	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:
		switch (chan->channel)
		{
		case 0:
			st->tmp_chan[0] = val;
			break;
		case 1:
			st->tmp_chan[1] = val;
			break;
		case 2:
			st->tmp_chan[2] = val;
			break;
		case 3:
			st->tmp_chan[3] = val;
			break;
		case 4:
			st->tmp_chan[4] = val;
			break;
		case 5:
			st->tmp_chan[5] = val;
			break;
		default:
			return -EINVAL;
			break;
		}
		return 0;

	case IIO_CHAN_INFO_ENABLE:
		st->en = val;
		return 0;

	default:
		return -EINVAL;
	}
}

static int ad5592r_s_reg_access(struct iio_dev *indio_dev,
			      	unsigned reg,
			      	unsigned writeval,
			      	unsigned *readval)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	if (readval)
		return ad5592r_s_spi_read_ctl(st, reg, (u16 *)readval);
	
	return ad5592r_s_spi_write(st, reg, writeval);
}

static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
	.debugfs_reg_access = &ad5592r_s_reg_access,
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
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
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
	st->tmp_chan[0] = 0;
	st->tmp_chan[1] = 0;
	st->tmp_chan[2] = 0;
	st->tmp_chan[3] = 0;
	st->tmp_chan[4] = 0;
	st->tmp_chan[5] = 0;
	st->spi = spi;

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
	indio_dev->channels = ad5592r_s_channel;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_s_channel);

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
    .driver = {
	.name = "ad5592r_s",
    },
    .probe = ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);

 MODULE_AUTHOR("Thomas Bontya <thomasbontya@gmail.com>");
 MODULE_DESCRIPTION("Analog Devices ad5592r ADC Driver");
 MODULE_LICENSE("GPL v2");