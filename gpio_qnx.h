// gpio_qnx.h
#pragma once
int gpio_init_output(int pin);
int gpio_init_input_pullup(int pin);
int gpio_write(int pin, int value);   // value 0/1
int gpio_read(int pin);              // returns 0/1
