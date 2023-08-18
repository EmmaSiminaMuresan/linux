// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI AD5592R driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <asm/unaligned.h>
#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/iio/iio.h>
#include <linux/delay.h>

#define AD5592R_S_REG_ADC_SEQ           0x2
#define AD5592R_S_REG_ADC_CFG           0x4
#define AD5592R_S_REG_READBACK          0x7
#define AD5592R_S_MASK_RDB_EN           BIT(6)
#define AD5592R_S_MASK_RDB_REG          GENMASK(5, 2)
#define AD5592R_S_ADDR_MASK_W           GENMASK(14, 11)
#define AD5592R_S_VAL_MASK_W            GENMASK(10, 0)
#define AD5592R_S_ADC_CHAN(x)           BIT(x)
#define AD5592R_S_ADC_ADDR_MASK         GENMASK(14, 12)
#define AD5592R_S_ADC_VAL_MASK          GENMASK(11, 0)
#define AD5592R_S_ADC_DEF_PIN_MASK      GENMASK(5, 0)
#define AD5592R_S_REG_REF_CFG           0x8
#define AD5592R_S_MASK_REF_EN		BIT(9)
#define AD5592R_S_REG_SW_RST		0xF
#define AD5592R_S_MASK_RST		0x5AC  

struct ad5592r_s_state {
        struct spi_device *spi;
        bool en;
        u16 tmp_chan[6];
};

static int ad5592r_s_spi_write(struct ad5592r_s_state *st, u8 reg, u16 val)
{
        u16 msg = 0;
        u16 tx  = 0;
        struct spi_transfer xfer = {
                .tx_buf = &tx,
                .len    = 2,
        };
        msg |= FIELD_PREP(AD5592R_S_ADDR_MASK_W, reg);
        msg |= FIELD_PREP(AD5592R_S_VAL_MASK_W,  val);
        //MSB first - BigEndian
        put_unaligned_be16(msg, &tx);
        return spi_sync_transfer(st->spi, &xfer, 1);
}

static int ad5592r_s_spi_nop(struct ad5592r_s_state *st, u16 *val)
{
        struct spi_transfer xfer = {
                        .tx_buf    = 0,
                        .rx_buf    = val,
                        .len       = 2,
        };
        return spi_sync_transfer(st->spi, &xfer, 1);
}

static int ad5592r_s_spi_read_ctl(struct ad5592r_s_state *st, u8 reg, u16 *val)
{
        u16 msg = 0;
        u16 tx  = 0;
        u16 rx  = 0;
        int ret;
        struct spi_transfer xfer = {
                .tx_buf    = &tx,
                .len       = 2,
        };
        msg |= FIELD_PREP(AD5592R_S_ADDR_MASK_W, AD5592R_S_REG_READBACK);
        msg |= AD5592R_S_MASK_RDB_EN;
        msg |= FIELD_PREP(AD5592R_S_MASK_RDB_REG, reg);
        //MSB first - BigEndian
        put_unaligned_be16(msg, &tx);
        ret = spi_sync_transfer(st->spi, &xfer, 1);
        if (ret) {
                dev_err(&st->spi->dev, "Failed at SPI WR transfer");
                return ret;
        }
        ret = ad5592r_s_spi_nop(st, &rx);
        if (ret) {
                dev_err(&st->spi->dev, "Failed at SPI WR NOP transfer");
                return ret;
        }
        *val = get_unaligned_be16(&rx);
        return 0;
}

static int ad5592r_s_spi_read_adc(struct ad5592r_s_state *st, u8 chan, int *val)
{
        u16 msg = 0;
        u16 tx  = 0;
        u16 rx  = 0;
        u16 tmp;
        u16 addr;
        int ret;
        struct spi_transfer xfer = {
                .tx_buf    = &tx,
                .len       = 2,
        };
        msg |= FIELD_PREP(AD5592R_S_ADDR_MASK_W, AD5592R_S_REG_ADC_SEQ);
        msg |= AD5592R_S_ADC_CHAN(chan);
        //MSB first - BigEndian
        put_unaligned_be16(msg, &tx);
        ret = spi_sync_transfer(st->spi, &xfer, 1);
        if (ret) {
                dev_err(&st->spi->dev, "Failed at SPI WR transfer");
                return ret;
        }
        ret = ad5592r_s_spi_nop(st, NULL);
        if (ret) {
                dev_err(&st->spi->dev, "Failed at SPI WR NOP transfer");
                return ret;
        }
                ret = ad5592r_s_spi_nop(st, &rx);
        if (ret) {
                dev_err(&st->spi->dev, "Failed at SPI WR NOP transfer");
                return ret;
        }
        tmp = get_unaligned_be16(&rx);
        addr = tmp;
        addr &= AD5592R_S_ADC_ADDR_MASK;
        addr = (addr >> 12);
        dev_info(&st->spi->dev, "req chan = 0x%x; recv chan = 0x%x", chan, addr);
        if (addr != chan) {
                dev_err(&st->spi->dev, "requested channel doesn't match read channel");
                return -EINVAL;
        }
        tmp &= AD5592R_S_ADC_VAL_MASK;
        *val = tmp;
        return 0;
}

static int ad5592r_s_read_raw(struct iio_dev *indio_dev,
                            struct iio_chan_spec const *chan,
                            int *val,
                            int *val2,
                            long mask)
{
        struct ad5592r_s_state *st = iio_priv(indio_dev);
        int ret;
        switch (mask) {
        case IIO_CHAN_INFO_RAW:
                ret = ad5592r_s_spi_read_adc(st, chan->channel, val);
                if (ret) {
                        dev_err(&st->spi->dev, "failed at read chan %d", chan->channel);
                        return ret;
                }
                return IIO_VAL_INT;
        case IIO_CHAN_INFO_ENABLE:
                *val = st->en;
                return IIO_VAL_INT;
        default:
                return -EINVAL; //error - Invalid Value
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
                        return -EINVAL; //error - Invalid Value
                }
                return 0;
        case IIO_CHAN_INFO_ENABLE:
                st->en = val;
                return 0;
        default:
                return -EINVAL; //error - Invalid Value
        }
}

static int ad5592r_s_reg_access(struct iio_dev *indio_dev,
                                unsigned reg, unsigned writeval,
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

static const struct iio_chan_spec ad5592r_channel[] = {
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
                // apare o singura data, daca este declarat la fiecare canal,
                // va fi ignorat
                .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
        },
        {
                .type = IIO_VOLTAGE,
                .channel = 2,
                .indexed = 1,
                .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                // apare o singura data, daca este declarat la fiecare canal,
                // va fi ignorat
                .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
        },
        {
                .type = IIO_VOLTAGE,
                .channel = 3,
                .indexed = 1,
                .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                // apare o singura data, daca este declarat la fiecare canal,
                // va fi ignorat
                .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
        },
        {
                .type = IIO_VOLTAGE,
                .channel = 4,
                .indexed = 1,
                .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                // apare o singura data, daca este declarat la fiecare canal,
                // va fi ignorat
                .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
        },
        {
                .type = IIO_VOLTAGE,
                .channel = 5,
                .indexed = 1,
                .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                // apare o singura data, daca este declarat la fiecare canal,
                // va fi ignorat
                .info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE),
        },
};

static int ad5592r_s_int(struct iio_dev *indio_dev)
{
        struct ad5592r_s_state *st = iio_priv(indio_dev);
        int ret;

	ret = ad5592r_s_spi_write(st, AD5592R_S_REG_SW_RST,
                                    AD5592R_S_MASK_RST);
	if (ret) {
                return ret;
        }
	usleep_range(250,300);

        ret = ad5592r_s_spi_write(st, AD5592R_S_REG_ADC_CFG,
                                    AD5592R_S_ADC_DEF_PIN_MASK);
        if (ret) {
                return ret;
        }

	ret = ad5592r_s_spi_write(st, AD5592R_S_REG_REF_CFG,
                                    AD5592R_S_MASK_REF_EN);
	if (ret) {
                return ret;
        }

        return 0;
}

static int ad5592r_s_probe(struct spi_device *spi)
{
        struct iio_dev *indio_dev;
        struct ad5592r_s_state *st;
        int ret;

        indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
        if (!indio_dev)
                return -ENOMEM; //eroare: Out Of Memory
        st = iio_priv(indio_dev);
        st->spi = spi;
        st->en = 0;
        st->tmp_chan[0] = 0;
        st->tmp_chan[1] = 0;
        st->tmp_chan[2] = 0;
        st->tmp_chan[3] = 0;
        st->tmp_chan[4] = 0;
        st->tmp_chan[5] = 0;
        indio_dev->name = "ad5592r_s";
        indio_dev->info = &ad5592r_s_info;
        indio_dev->channels = ad5592r_channel;
        indio_dev->num_channels = ARRAY_SIZE(ad5592r_channel);
        ret = ad5592r_s_int(indio_dev);
        if (ret) {
                dev_err(&spi->dev, "Failed at init");
                return ret;
        }
        return devm_iio_device_register(&spi->dev, indio_dev);
}

static struct spi_driver ad5592r_s_driver = {
        .driver = {
                .name = "ad5592r_s",
        },
        .probe =  ad5592r_s_probe,
};

module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR("Vlad Cristescu <vladcristescu1@gmail.com>");
MODULE_DESCRIPTION("Analog Devices AD5592r driver");
MODULE_LICENSE("GPL 0v2");