// SPDX-License-Identifier: GPL-2.0-only
/*
 * ADI ad5592r ADC Converter driver
 *
 * Copyright 2023 Analog Devices Inc.
 */

#include <linux/module.h>
#include <linux/spi/spi.h>

#include <linux/iio/iio.h>

static int ad5592r_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val,
			    int *val2,
			    long mask)
{
	switch (mask)
	{
	case IIO_CHAN_INFO_RAW:
	        switch (chan->channel)
		{
		case 0: *val = 0;
		         break;

		case 1: *val = 1;
                        break;
		case 2: *val = 2;
                        break;
		case 3: *val = 3;
                        break;
		case 4: *val = 4;
                        break;
		case 5: *val = 5;
                        break;
	       }
		return 	IIO_VAL_INT;	
	default:
		return -EINVAL;
	}
}


static const struct iio_info  ad5592r_info = {
     
     .read_raw = &ad5592r_read_raw,

};


static const struct iio_chan_spec ad5592r_channels[] = {

     {	 
       .type = IIO_VOLTAGE,
       .channel = 0,
       .indexed =1,
       .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
     },

    {	 
       .type = IIO_VOLTAGE,
       .channel = 1,
       .indexed =1,
       .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
     },

     {	 
       .type = IIO_VOLTAGE,
       .channel = 2,
       .indexed =1,
       .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
     },

     {	 
       .type = IIO_VOLTAGE,
       .channel = 3,
       .indexed =1,
       .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
     },

     {	 
       .type = IIO_VOLTAGE,
       .channel = 4,
       .indexed =1,
       .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
     },

     {	 
       .type = IIO_VOLTAGE,
       .channel = 5,
       .indexed =1,
       .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
     },


};


static int ad5592r_probe(struct spi_device *spi)
{
	struct iio_dev *indio_dev;
        
	indio_dev = devm_iio_device_alloc(&spi->dev, 0);
	if(!indio_dev)
	           return  -ENOMEM;
       
        indio_dev->name = "ad5592r_s";
	indio_dev->info =  &ad5592r_info;
        indio_dev->channels = ad5592r_channels;
	indio_dev->num_channels =ARRAY_SIZE(ad5592r_channels);

        
	return devm_iio_device_register(&spi->dev,indio_dev);
}



static struct spi_driver ad5592r_driver = {

    .driver = {
	    .name = "ad5592r_sv",
    },
       .probe =  ad5592r_probe,

  
};
module_spi_driver(ad5592r_driver);

MODULE_AUTHOR("Valentin Dolcescu <valentindolcescu.vd@gmail.com>");
MODULE_DESCRIPTION("Analog Devices ad5592r ADC Converter Driver");
MODULE_LICENSE("GPL v2");



