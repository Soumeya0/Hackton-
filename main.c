#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "traffic_light.h"

static volatile int running = 1;

static void handle(int sig) {
    (void)sig;
    running = 0;
}

int main(void) {
    signal(SIGINT, handle);
    signal(SIGTERM, handle);

    init_gpio();
    printf("Running. Press the physical button to request crossing. Ctrl+C to stop.\n");

    while (running) {
        update_traffic_light();
        sleep(1);
    }

    cleanup_gpio();
    printf("Exited cleanly.\n");
    return 0;
}
