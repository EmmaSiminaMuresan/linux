// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for AD5592R ADC
 *
 * Copyright (c) 2023 Analog Devices
 */

#include <asm/unaligned.h>
#include <linux/bitfield.h>

#include <linux/module.h>
#include <linux/iio/iio.h>
#include <linux/spi/spi.h>

#define ADC_SEQ 0x2

#define ADI_WR_REG_MASK 	GENMASK(14, 11)
#define ADI_WR_VAL_MASK 	GENMASK(8, 0)
#define ADI_RD_VAL_MASK		GENMASK(11,0)


struct adi_ad5592r_s_state {
	struct spi_device* spi;

	bool en;
	u16 temp_chann [6];
};

static int adi_ad5592r_s_spi_write(struct adi_ad5592r_s_state *st,
			     u8 reg,
			     u16 val)
{
	// b15 always 0 for write
	u16 msg = 0;
	u16 tx = 0;

	struct spi_transfer xfer = {
		.tx_buf = NULL,
		.len = 2,
	};

	msg |= FIELD_PREP(ADI_WR_REG_MASK, reg);
	msg |= FIELD_PREP(ADI_WR_VAL_MASK, val);

	put_unaligned_be16(msg, &tx);
	xfer.tx_buf = &tx;

	return spi_sync_transfer(st->spi, &xfer, 1);
}

static int adi_ad5592r_s_spi_red(struct adi_ad5592r_s_state *st,
			     u8 reg,
			     u16 *val)
{
	u16 tx = 0; //addr we want to read from
	u16 rx = 0;
	u16 msg = 0;
	int ret;

	struct spi_transfer xfer[] = {
		{
			.tx_buf = NULL,
			.rx_buf = NULL,
			.len = 2,
		},
		{
			.tx_buf = NULL,
			.rx_buf = NULL,
			.len = 2,
		},
		{
			.tx_buf = NULL,
			.rx_buf = NULL,
			.len = 2,
		},
	};


	// Select ADC channel to read from
	msg |= FIELD_PREP(ADI_WR_REG_MASK, ADC_SEQ);
	msg |= BIT(reg);

	put_unaligned_be16(msg, &tx);
	xfer[0].tx_buf = &tx;

	// NOOP
	tx = 0;
	xfer[1].tx_buf = &tx;

	// read value recieved

	xfer[2].rx_buf = &rx;
	rx &= ADI_RD_VAL_MASK;


	ret = spi_sync_transfer(st->spi, xfer, 3);

	if (ret)
		return ret;

	*val = rx;
	return 0;
}

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

static int adi_emu_reg_access(struct iio_dev *indio_dev, // return by iio 
			      unsigned int reg,
			      unsigned int writeval,
			      unsigned int *readval)
{
	struct adi_ad5592r_s_state *st = iio_priv(indio_dev);

	if (readval) 
		return adi_ad5592r_s_spi_read(st, reg, (u8 *)readval);

	return  adi_ad5592r_s_spi_write(st, reg, writeval);
}

static const struct iio_info adi_ad5592r_s_info = {
	.read_raw =  &adi_ad5592r_s_read_raw,
	.write_raw = &adi_ad5592r_s_write_raw,
	.debugfs_reg_access = &adi_emu_reg_access,
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
	st->spi = spi;
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

