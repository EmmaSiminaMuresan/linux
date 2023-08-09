// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI AD5592R ADC driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

struct ad5592r_s_state {
	bool en;
	u16 tmp_chan0;
	u16 tmp_chan1;
	u16 tmp_chan2;
	u16 tmp_chan3;
	u16 tmp_chan4;
	u16 tmp_chan5;

};

static int ad5592r_s_read_raw(struct iio_dev *indio_dev, 
			                struct iio_chan_spec const *chan,
			                int *val,
			                int *val2,
			                long mask)
{ 
  struct ad5592r_s_state *st = iio_priv(indio_dev);
  
  switch (mask){
  case IIO_CHAN_INFO_RAW:
	  switch (chan->channel)
	  {
	  case 0:
		  *val = st->tmp_chan0;
		  break;
	  case 1:
		  *val = st->tmp_chan1;
		  break;
	  case 2:
		  *val = st->tmp_chan2;
		  break;
	  case 3:
	      *val = st->tmp_chan3;
		  break;	  	  	  
	  case 4:
		  *val = st->tmp_chan4;
		  break;
	  case 5:
		  *val = st->tmp_chan5;
		  break;
	  default:
	      return  100;
		  break;
	  }	
	 return IIO_VAL_INT;
  case IIO_CHAN_INFO_ENABLE:
			*val  = st->en;
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

	switch (mask){
  	case IIO_CHAN_INFO_RAW:
	  switch (chan->channel)
	  {
	  case 0:
		  st->tmp_chan0 = val;
		  break;
	  case 1:
		  st->tmp_chan1 = val;
		  break;
	  case 2:
		  st->tmp_chan2 = val;
		  break;
	  case 3:
	      st->tmp_chan3 = val;
		  break;	  	  	  
	  case 4:
		  st->tmp_chan4 = val;
		  break;
	  case 5:
		  st->tmp_chan5 = val;
		  break;
	  default:
	      return  100;
		  break;
	  }	
	 	return 0;

  	case IIO_CHAN_INFO_ENABLE:
	  	st->en = val;
	  	return 0; // succes
	default: 
		return -EINVAL;
	}	

}


static const struct iio_info ad5592r_s_info = {
	.read_raw = &ad5592r_s_read_raw,
	.write_raw = &ad5592r_s_write_raw,
};

static const struct iio_chan_spec ad5592r_channel[] ={
	{
	// scriem un driver pentru un ADC	
	// ce contine structurscp arch/arm/boot/uImage root@10.76.84.205:/boot/uImagea
		.type = IIO_VOLTAGE,
		.channel = 0,
		.indexed = 1,  // ca sa puna numar la sfarsitul canalului
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_ENABLE), 
	},
	{
	
		.type = IIO_VOLTAGE,
		.channel = 1,
		.indexed = 1, 
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
	
		.type = IIO_VOLTAGE,
		.channel = 2,
		.indexed = 1,  
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
		{
	
		.type = IIO_VOLTAGE,
		.channel = 3,
		.indexed = 1,  
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
	{
	
		.type = IIO_VOLTAGE,
		.channel = 4,
		.indexed = 1,  
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
		{
	
		.type = IIO_VOLTAGE,
		.channel = 5,
		.indexed = 1,  
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
	},
};


static int ad5592r_s_probe(struct spi_device *spi)
{
         struct iio_dev *indio_dev;
		 struct ad5592r_s_state *st;

	 indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	 if (!indio_dev)
	 	return -ENOMEM; //mesaj de eroare Eroare de memorie

    st = iio_priv(indio_dev);
	st->en = 0; // valoarea 0 la inceput
	st->tmp_chan0 = 0;
	st->tmp_chan1 = 0;
	st->tmp_chan2 = 0;
	st->tmp_chan3 = 0;
	st->tmp_chan4 = 0;
	st->tmp_chan5 = 0;

	indio_dev->name = "ad5592r_s";
	indio_dev->info = &ad5592r_s_info;
    indio_dev->channels =  ad5592r_channel;
	indio_dev->num_channels = ARRAY_SIZE(ad5592r_channel);

	return devm_iio_device_register(&spi->dev,indio_dev);	 
}


static struct spi_driver ad5592r_s_driver = {
        .driver = {
			.name = "ad5592r_s", // similar cu nume fisier
	},
	.probe = ad5592r_s_probe,
};
module_spi_driver(ad5592r_s_driver);

MODULE_AUTHOR ("Pop Ioan Daniel <Pop.Ioan-daniel@analog.com>");
MODULE_DESCRIPTION("Analog Devices AD5592R Driver");
MODULE_LICENSE("GPL v2");