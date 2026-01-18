// gpio_qnx.c
#include "gpio_qnx.h"
#include <stdlib.h>
#include <stdio.h>

static int run(const char *cmd) {
    int rc = system(cmd);
    return rc;
}

int gpio_init_output(int pin){
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 set %d op", pin);
    return run(cmd);
}

int gpio_init_input_pullup(int pin){
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 set %d ip pu", pin);
    return run(cmd);
}

int gpio_write(int pin, int value){
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 set %d %s", pin, value ? "dh" : "dl");
    return run(cmd);
}

int gpio_read(int pin){
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 get %d", pin);
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;
    int v = -1;
    if (fscanf(fp, "%d", &v) != 1) v = -1;
    pclose(fp);
    return v;
}
