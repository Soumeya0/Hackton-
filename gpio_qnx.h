#ifndef GPIO_QNX_H
#define GPIO_QNX_H

// QNX GPIO wrapper using gpio-rp1
int gpio_init_output(int pin);
int gpio_init_input_pullup(int pin);
int gpio_write(int pin, int value);   // 0 = low, 1 = high
int gpio_read(int pin);              // returns 0 or 1

#endif
