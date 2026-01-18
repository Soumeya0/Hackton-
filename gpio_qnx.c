#include "gpio_qnx.h"
#include <stdio.h>
#include <stdlib.h>

static int run_cmd(const char *cmd) {
    return system(cmd);
}

int gpio_init_output(int pin) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 set %d op", pin);
    return run_cmd(cmd);
}

int gpio_init_input_pullup(int pin) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 set %d ip pu", pin);
    return run_cmd(cmd);
}

int gpio_write(int pin, int value) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 set %d %s", pin, value ? "dh" : "dl");
    return run_cmd(cmd);
}

int gpio_read(int pin) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "gpio-rp1 get %d", pin);
    FILE *fp = popen(cmd, "r");
    if (!fp) return -1;

    int v = -1;
    fscanf(fp, "%d", &v);
    pclose(fp);
    return v;
}
