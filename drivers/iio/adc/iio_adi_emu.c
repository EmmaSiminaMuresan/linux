// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI ADC emulator driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <asm/unaligned.h>
#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>


#define ADI_EMU_RD_MASK		BIT(7)
#define ADI_EMU_ADDR_MASK	GENMASK(14, 8)
#define ADI_EMU_VAL_MASK	GENMASK( 7, 0)

struct adi_emu_state
{
	struct spi_device *spi;
	bool en;
	u16 tmp_chan0;
	u16 tmp_chan1;
};

// Read from a register
static int adi_emu_spi_read(struct adi_emu_state *st, u8 reg, u8 *val)
{
	u8 tx = 0;
	u8 rx = 0;
	int ret = 0;
	
	// struct spi_transfer is used to describe a single SPI transfer
	struct spi_transfer xfer[] = {
		{
			.tx_buf = NULL,
			.rx_buf = NULL,
			.len = 1,
			// .cs_change = 1, // <- chip select doesn't change
					// between transfers. useful when
					// multiple transfers are done
		},
		{
			.tx_buf = NULL,
			.rx_buf = NULL,
			.len = 1,
		}
	};

	tx  = reg; // set register address from which to read
	tx |= ADI_EMU_RD_MASK; // set read bit
	dev_info(&st->spi->dev, "tx at read: 0x%x\n", tx);

	// set tx and rx buffers
	xfer[0].tx_buf = &tx;
	xfer[1].rx_buf = &rx;

	// make spi transfer
	ret = spi_sync_transfer(st->spi, xfer, 2);
	if (ret < 0)
		return ret;
	*val = rx;
	return 0;
}

// Write to a register
static int adi_emu_spi_write(struct adi_emu_state *st, u8 reg, u8 val)
{
	u16 msg = 0;
	u16 tx = 0;
	struct spi_transfer xfer = {
		.tx_buf = NULL,
		.len = 2, // 
	};

	// set register address and value
	msg  = FIELD_PREP(ADI_EMU_ADDR_MASK, reg);
	msg |= FIELD_PREP(ADI_EMU_VAL_MASK, val);
	dev_info(&st->spi->dev, "msg at write: 0x%x\n", msg);

	// convert to big endian
	put_unaligned_be16(msg, &tx);
	dev_info(&st->spi->dev, "tx at write: 0x%x\n", tx);

	xfer.tx_buf = &tx;
	return spi_sync_transfer(st->spi, &xfer, 1);
}

// Read channel and, depending on the channel, return a value
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

static int adi_emu_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val,
			     int val2,
			     long mask)
{
	struct adi_emu_state *st = iio_priv(indio_dev);
	switch (mask)
	{
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

static int adi_emu_reg_access(struct iio_dev *indio_dev,
			      unsigned reg,
			      unsigned writeval,
			      unsigned *readval)
{
	struct adi_emu_state *st = iio_priv(indio_dev);

	if (readval)
		return adi_emu_spi_read(st, reg, (u8 *)readval);
	
	return adi_emu_spi_write(st, reg, writeval);
}

static const struct iio_info adi_emu_info = {
	.read_raw = &adi_emu_read_raw,
	.write_raw = &adi_emu_write_raw,
	.debugfs_reg_access = &adi_emu_reg_access,
};

static const struct iio_chan_spec adi_emu_channel[] = {
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

static int adi_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct adi_emu_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));

	if (!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);
	st->en = 0;
	st->tmp_chan0 = 0;
	st->tmp_chan1 = 0;
	st->spi = spi;

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

MODULE_AUTHOR("Tudor Radoni <tudor.radoni@gmail.com>");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");
