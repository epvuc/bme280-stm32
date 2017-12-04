# bme280-stm32

The file "sup.c" implements the three platform-specific functions required to use the reference code from Bosch:
user_i2c_read(), user_i2c_write(), and user_delay_ms().  The latter assumes a 1000 hz systick but timing doesn't
appear to be really critical.

It also contains my_print_readings() which initializes the bme280_dev structure, calls the bme280_init() function,
and then goes to the example function from the reference code for fetching and printing measurements in a loop.

