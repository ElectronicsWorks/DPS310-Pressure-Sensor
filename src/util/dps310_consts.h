/**
 * Arduino library to control Dps310
 *
 * "Dps310" represents Infineon's high-sensetive pressure and temperature sensor. 
 * It measures in ranges of 300 - 1200 hPa and -40 and 85 °C. 
 * The sensor can be connected via SPI or I2C. 
 * It is able to perform single measurements
 * or to perform continuous measurements of temperature and pressure at the same time, 
 * and stores the results in a FIFO to reduce bus communication. 
 *
 * Have a look at the datasheet for more information. 
 */

#ifndef DPS310_CONSTS_H_
#define DPS310_CONSTS_H_


	//general Constants
#define DPS310__PROD_ID						0U
#define DPS310__STD_SLAVE_ADDRESS 			0x77U
#define DPS310__SPI_WRITE_CMD 				0x00U
#define DPS310__SPI_READ_CMD 				0x80U
#define DPS310__SPI_RW_MASK 				0x80U
#define DPS310__SPI_MAX_FREQ 				100000U

#define DPS310__LSB 0x01U

#define DPS310__TEMP_STD_MR					2U
#define DPS310__TEMP_STD_OSR				3U
#define DPS310__PRS_STD_MR					2U
#define DPS310__PRS_STD_OSR					3U
#define DPS310__OSR_SE 						3U
//we use 0.1 mS units for time calculations, so 10 units are one millisecond
#define DPS310__BUSYTIME_SCALING 			10U
// DPS310 has 10 milliseconds of spare time for each synchronous measurement / per second for asynchronous measurements
// this is for error prevention on friday-afternoon-products :D
// you can set it to 0 if you dare, but there is no warranty that it will still work
#define DPS310__BUSYTIME_FAILSAFE			10U
#define DPS310__MAX_BUSYTIME 				((1000U-DPS310__BUSYTIME_FAILSAFE)*DPS310__BUSYTIME_SCALING)
#define DPS310__NUM_OF_SCAL_FACTS			8

#define DPS310__SUCCEEDED 					0
#define DPS310__FAIL_UNKNOWN 				-1
#define DPS310__FAIL_INIT_FAILED 			-2
#define DPS310__FAIL_TOOBUSY 				-3
#define DPS310__FAIL_UNFINISHED 			-4

#define DPS310__REG_ADR_SPI3W 				0x09U
#define DPS310__REG_CONTENT_SPI3W 			0x01U
#endif /* DPS310_CONSTS_H_ */
