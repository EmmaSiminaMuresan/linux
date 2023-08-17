// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ADI ADC Emulated driver
 *
 * Copyright (C) 2023 Analog Devices, Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/bitfield.h>
#include <asm/unaligned.h>

#define ADI_EMU_RD_MASK 	BIT(7)
#define ADI_EMU_ADDR_MASK	GENMASK(14,8)
#define ADI_EMU_VAL_MASK	GENMASK(7, 0)

#define ADI_EMU_REG_CNVST	0x3
#define ADI_EMU_MASK_CNVST	BIT(0)

#define ADI_EMU_REG_CH0_DATA_LOW	0x4
#define ADI_EMU_REG_CH0_DATA_HIGH	0x5

#define ADI_EMU_REG_CH1_DATA_LOW	0x6
#define ADI_EMU_REG_CH1_DATA_HIGH	0x7

#define ADI_EMU_REG_DEVICE_CONFIG	0x2
#define ADI_EMU_MASK_PWD		BIT(5)

struct adi_emu_state{
	bool en;
	


	struct spi_device *spi;
};

static int adi_emu_spi_read(struct adi_emu_state *st, u8 reg, u8 *val)
{
	u8 tx = 0;
	u8 rx = 0;
	int ret; 

	struct spi_transfer xfer[] = {
		{
			.tx_buf = NULL,
			.rx_buf = NULL,
			.len = 1,
			.cs_change = 1,
		},
		{
			.tx_buf = NULL,
			.rx_buf = NULL,
			.len = 1,
		}
	};

	tx = reg;
	tx |= ADI_EMU_RD_MASK;

	dev_info(&st->spi->dev, "tx at read = 0%x", tx);

	xfer[0].tx_buf = &tx;
	xfer[1].rx_buf = &rx;

	ret = spi_sync_transfer(st->spi, xfer, 2);

	if(ret)
		return ret;
	
	*val = rx;

	return 0;
}

static int adi_emu_spi_write(struct adi_emu_state *st, u8 reg, u8 val)
{
	u16 msg = 0;
	u16 tx = 0;
	struct spi_transfer xfer ={
		.tx_buf = NULL,
		.len = 2,
	};

	msg |= FIELD_PREP(ADI_EMU_ADDR_MASK, reg);
	msg |= FIELD_PREP(ADI_EMU_VAL_MASK, val);

	dev_info(&st->spi->dev, "msg = 0%x", msg);

	put_unaligned_be16(msg, &tx);
	
	dev_info(&st->spi->dev, "tx at write = 0%x", tx);

	xfer.tx_buf = &tx;
	
	return spi_sync_transfer(st->spi, &xfer, 1);
}

static int adi_emu_read_adc(struct adi_emu_state *st, int chan, int *val)
{
	int ret;
	u8 high;
	u8 low;

	ret = adi_emu_spi_write(st, 
				ADI_EMU_REG_CNVST, 
				ADI_EMU_MASK_CNVST);

	if(ret){
		dev_err(&st->spi->dev, 
		"Error at conversion");
		return ret;
	}

	if(chan)
	{
		ret = adi_emu_spi_read(st, 
				ADI_EMU_REG_CH1_DATA_HIGH,
				&high);
		if(ret){
			dev_err(&st->spi->dev, 
			"Error at ch1 high read");
			return ret;
		}

		ret = adi_emu_spi_read(st, 
				ADI_EMU_REG_CH1_DATA_LOW,
				&low);
		if(ret){
			dev_err(&st->spi->dev, 
			"Error at ch1 low read");
			return ret;
		}
	} else {
		ret = adi_emu_spi_read(st, 
				ADI_EMU_REG_CH0_DATA_HIGH,
				&high);
		if(ret){
			dev_err(&st->spi->dev, 
			"Error at ch0 high read");
			return ret;
		}

		ret = adi_emu_spi_read(st, 
				ADI_EMU_REG_CH0_DATA_LOW,
				&low);
		if(ret){
			dev_err(&st->spi->dev, 
			"Error at ch0 low read");
			return ret;
		}
	}
	
	*val = (high << 8) | low;

	return 0;
}

static int adi_emu_read_raw(struct iio_dev *indio_dev,
			struct iio_chan_spec const *chan,
			int *val,
			int *val2,
			long mask)
{
	struct adi_emu_state *st = iio_priv(indio_dev);
	
	int ret;

	switch(mask){
		case IIO_CHAN_INFO_RAW:

			ret = adi_emu_read_adc(st, chan->channel, val);
			if(ret){
				dev_err(&st->spi->dev, 
				"Error at read adc %d", chan->channel);
				return ret;
			}
						
			return IIO_VAL_INT;
		case IIO_CHAN_INFO_ENABLE:
			*val = st->en;
			return IIO_VAL_INT;
		default:
			return -EINVAL;
	}
};


static int adi_emu_write_raw(struct iio_dev *indio_dev,
			     struct iio_chan_spec const *chan,
			     int val,
			     int val2,
			     long mask)
{
	struct adi_emu_state *st = iio_priv(indio_dev);
	int ret;
		
	switch (mask)
	{
		case IIO_CHAN_INFO_ENABLE:
			st->en = val;
			ret =  adi_emu_spi_write(st, 
					ADI_EMU_REG_DEVICE_CONFIG, 
					FIELD_PREP(ADI_EMU_MASK_PWD, !val));
			if(ret){
				dev_err(&st->spi->dev, 
				"Error at writting enable");
				return ret;
			}
			return 0;
		default:
			return -EINVAL;
	}
};

static int adi_emu_reg_acces(struct iio_dev *indio_dev,
				unsigned reg, unsigned writeval,
				unsigned *readval)
{
	struct adi_emu_state *st = iio_priv(indio_dev);

	if(readval)
		return adi_emu_spi_read(st, reg, (u8 *)readval);

	return adi_emu_spi_write(st, reg, writeval);	
}

static const struct iio_info adi_emu_info = {
	.read_raw = &adi_emu_read_raw,
	.write_raw = &adi_emu_write_raw,
	.debugfs_reg_access = &adi_emu_reg_acces,
};

static const struct iio_chan_spec adi_emu_channel[] ={
	{
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE), 
		//suficient sa scriem pt un singur canal
	},
	{
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	}
};



static int adi_emu_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
	struct adi_emu_state *st;

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if(!indio_dev)
		return -ENOMEM;
	
	st = iio_priv(indio_dev);
	st->spi = spi;
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

module_spi_driver (adi_emu_driver);

MODULE_AUTHOR("Popa Cristian Mihai <Cristianmihai.Popa@analog.com");
MODULE_DESCRIPTION("Analog Devices ADC Emulator Driver");
MODULE_LICENSE("GPL v2");