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

#define AD5592R_REG_ADC_SEQ	 	0x2
#define AD5592R_REG_ADC_CONFIG	 	0x4


#define AD5592R_REG_READBACK_AND_LDAC	0x7
#define AD5592R_MASK_RDB_REG_GENMASK	GENMASK(5, 2)
#define AD5592R_MASK_RDB_EN		BIT(6)


#define AD5592R_REG_MASK	GENMASK(14, 11)
#define AD5592R_VAL_MASK	GENMASK(10, 0)


#define AD5592R_VAL_RD_MASK	GENMASK(11,0)



#define ADI_ADC_EN_MASK		GENMASK(5,0)

enum Channels {
	CH0 = 0,
	CH1,
	CH2,
	CH3,
	CH4,
	CH5,
};


struct ad5592r_s_state {
	struct spi_device* spi;

	bool en;
	u16 temp_chann [6];
};

static int ad5592r_s_spi_write(struct ad5592r_s_state *st,
			     u8 reg,
			     u16 val)
{
	u16 msg = 0;
	u16 tx = 0;

	struct spi_transfer xfer = {
		.tx_buf = &tx,
		.len = 2,
	};

	msg |= FIELD_PREP(AD5592R_REG_MASK, reg);
	msg |= FIELD_PREP(AD5592R_VAL_MASK, val);

	put_unaligned_be16(msg, &tx);

	dev_info(&st->spi->dev, "Write, TX = %x", tx);

	return spi_sync_transfer(st->spi, &xfer, 1);
}

static int ad5592r_s_spi_read_noop(struct ad5592r_s_state *st,
			     	   u16 *val)
{
	struct spi_transfer xfer = {
		.tx_buf = 0,
		.rx_buf = val,
		.len = 2,
	};
	
	return spi_sync_transfer(st->spi, &xfer, 1);
}


static int ad5592r_s_spi_read_ctl(struct ad5592r_s_state *st,
			     u8 reg,
			     u16 *val)
{
	u16 tx = 0; //addr we want to read from
	u16 rx = 0;
	u16 msg = 0;
	
	int ret;

	struct spi_transfer xfer = {
		.tx_buf = &tx,
		.len = 2,
	};

	dev_info(&st->spi->dev, "DEBUG REG READ:(reg = %d)", reg);


	// 1. configure READ_AND_LDAC register for readback
	msg |= FIELD_PREP(AD5592R_REG_MASK, AD5592R_REG_READBACK_AND_LDAC);
	msg |= FIELD_PREP(AD5592R_MASK_RDB_REG_GENMASK, reg);
	msg |= AD5592R_MASK_RDB_EN; //// b6 = 1

	put_unaligned_be16(msg, &tx);

	dev_info(&st->spi->dev, "DEBUG REG READ:(step1 readback reg val = %x)", reg);


	ret = spi_sync_transfer(st->spi, &xfer, 1);
	if (ret)
	{
		dev_err(&st->spi->dev, "Failed at SPI WR transfer");
		return ret;
	}

	ret = ad5592r_s_spi_read_noop(st, &rx);
	if (ret)
	{
		dev_err(&st->spi->dev, "Failed at SPI WR NOOP transfer");
		return ret;
	}	

	dev_info(&st->spi->dev, "DEBUG REG READ:(return val = %x)", rx);
	*val = get_unaligned_be16(&rx);
	return 0;

}

// static int adi_ad5592r_s_spi_read_adc(struct adi_ad5592r_s_state *st,
// 			     u8 reg,
// 			     u16 *val)
// {
// 	u16 tx = 0; //addr we want to read from
// 	u16 rx = 0;
// 	u16 msg = 0;
// 	int ret;

// 	struct spi_transfer xfer[] = {
// 		{
// 			.tx_buf = NULL,
// 			.rx_buf = NULL,
// 			.len = 2,
// 		},
// 		{
// 			.tx_buf = NULL,
// 			.rx_buf = NULL,
// 			.len = 2,
// 		},
// 		{
// 			.tx_buf = NULL,
// 			.rx_buf = NULL,
// 			.len = 2,
// 		},
// 	};


// 	// Select ADC channel to read from
// 	msg |= FIELD_PREP(ADI_WR_REG_MASK, AD5592R_REG_ADC_SEQ);
// 	// msg |= BIT(reg); // varianta initiala pe git
// 	msg |= FIELD_PREP(ADI_WR_VAL_MASK, reg);

// 	put_unaligned_be16(msg, &tx);
// 	xfer[0].tx_buf = &tx;

// 	dev_info(&st->spi->dev, "Read step1: TX = %x", tx);

// 	// NOOP
// 	tx = 0;
// 	xfer[1].tx_buf = &tx;

// 	dev_info(&st->spi->dev, "Read step2(NOOP): TX = %x", tx);


// 	// read value recieved
// 	xfer[2].rx_buf = &rx;

// 	dev_info(&st->spi->dev, "Read step3(DATA-reg): RX = %x", rx);

// 	rx &= ADI_RD_VAL_MASK;

// 	dev_info(&st->spi->dev, "Read step3(DATA-masked): RX = %x", rx);


// 	ret = spi_sync_transfer(st->spi, xfer, 3);

// 	if (ret)
// 		return ret;

// 	*val = rx;
// 	return 0;
// }

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
		case CH0:
			*val = st->temp_chann[0];
			break;
		case CH1:
			*val = st->temp_chann[1];
			break;
		case CH2:
			*val = st->temp_chann[2];
			break;
		case CH3:
			*val = st->temp_chann[3];
			break;
		case CH4:
			*val = st->temp_chann[4];
			break;
		case CH5:
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
		case CH0:
			st->temp_chann[0]= val;
			break;
		case CH1:
			st->temp_chann[1]= val;
			break;
		case CH2:
			st->temp_chann[2]= val;
			break;
		case CH3:
			st->temp_chann[3]= val;
			break;
		case CH4:
			st->temp_chann[4]= val;
			break;
		case CH5:
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

static int ad5592r_s_reg_access(struct iio_dev *indio_dev, // return by iio 
			      unsigned int reg,
			      unsigned int writeval,
			      unsigned int *readval)
{
	struct ad5592r_s_state *st = iio_priv(indio_dev);

	if (readval) 
		return ad5592r_s_spi_read_ctl(st, reg, (u16 *)readval);

	return  ad5592r_s_spi_write(st, reg, writeval);
}

static const struct iio_info adi_ad5592r_s_info = {
	.read_raw =  &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
	.debugfs_reg_access = &ad5592r_s_reg_access,
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

// static int adi_ad5592r_s_init(struct iio_dev *indio_dev)
// {
// 	// Set channels [5;0] as ADC inputs
// 	// return adi_emu_reg_access(indio_dev, reg, writeval, NULL);
// 	int ret;
// 	struct adi_ad5592r_s_state *st = iio_priv(indio_dev);
// 	// configuree ADC          channels
// 	ret =  adi_ad5592r_s_spi_write(st, AD5592R_REG_ADC_CONFIG, ADI_ADC_EN_MASK);

// 	if (ret)
// 		return ret;
// 	return 0;
// }

static int ad5592r_s_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct ad5592r_s_state *st;
	// int ret;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if(!indio_dev)
		return -ENOMEM;

	st = iio_priv(indio_dev);
	st->spi = spi;
	spi->mode = 3;

	st->en = 0;
	st->temp_chann[0] = 0;
	st->temp_chann[1] = 0;
	st->temp_chann[2] = 0;
	st->temp_chann[3] = 0;
	st->temp_chann[4] = 0;
	st->temp_chann[5] = 0;

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &adi_ad5592r_s_info;
	indio_dev->channels = adi_5592r_s_channels;
	indio_dev->num_channels = ARRAY_SIZE(adi_5592r_s_channels);

	// ret = adi_ad5592r_s_init(indio_dev);
	// if (ret)
	// 	return ret;

	return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver adi_ad5592r_s_driver = {
	.driver = {
		.name = "adi_ad5592r_s",
	},
	.probe = ad5592r_s_probe,
};

module_spi_driver(adi_ad5592r_s_driver); // inregistreaza modulul ca driver de SPI

MODULE_AUTHOR("Bogdan Stanea <Adrian.Stanea@analog.com>");
MODULE_DESCRIPTION("Analog Devices driver for AD5592R ADC");
MODULE_LICENSE("GPL v2");

