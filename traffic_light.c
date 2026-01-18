#include "traffic_light.h"
#include "gpio_qnx.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>

static TLState state = STATE_RED;
static int t = 0; // seconds elapsed in current state

static int ped_requested = 0;
static uint64_t last_press_ms = 0;

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
}

static void set_leds(int r, int y, int g) {
    gpio_write(RED_PIN, r);
    gpio_write(YELLOW_PIN, y);
    gpio_write(GREEN_PIN, g);
}

static void apply_state(TLState s) {
    switch (s) {
        case STATE_GREEN:   set_leds(0,0,1); break;
        case STATE_YELLOW:  set_leds(0,1,0); break;
        case STATE_RED:     set_leds(1,0,0); break;
        case STATE_PED_RED: set_leds(1,0,0); break; // same LEDs as RED (cars stopped)
    }
}

void init_gpio(void) {
    gpio_init_output(RED_PIN);
    gpio_init_output(YELLOW_PIN);
    gpio_init_output(GREEN_PIN);
    gpio_init_input_pullup(PED_BUTTON_PIN);

    set_leds(0,0,0);

    state = STATE_RED;
    t = 0;
    ped_requested = 0;

    apply_state(state);

    printf("GPIO ready. Button GPIO%d (pull-up, active LOW)\n", PED_BUTTON_PIN);
}

void cleanup_gpio(void) {
    set_leds(0,0,0);
}

TLState get_state(void) {
    return state;
}

// returns 1 if a *new* press is detected
static int check_button_pressed(void) {
    static int last = 1; // idle HIGH
    int cur = gpio_read(PED_BUTTON_PIN);
    uint64_t ms = now_ms();

    int pressed = 0;
    if (cur == 0 && last == 1) {
        if (ms - last_press_ms > BUTTON_DEBOUNCE_MS) {
            pressed = 1;
            last_press_ms = ms;
        }
    }
    last = cur;
    return pressed;
}

void update_traffic_light(void) {
    // button press => accelerate to RED sooner
    if (check_button_pressed()) {
        if (!ped_requested && state != STATE_PED_RED) {
            ped_requested = 1;
            printf("[BTN] Pedestrian requested: accelerating to RED.\n");
        }
    }

    t++;

    // Choose durations (accelerated if ped requested and we're not in red/ped yet)
    int green_d  = (ped_requested ? ACCEL_GREEN_DURATION  : GREEN_DURATION);
    int yellow_d = (ped_requested ? ACCEL_YELLOW_DURATION : YELLOW_DURATION);

    switch (state) {
        case STATE_GREEN:
            if (t >= green_d) {
                state = STATE_YELLOW;
                t = 0;
                apply_state(state);
            }
            break;

        case STATE_YELLOW:
            if (t >= yellow_d) {
                state = STATE_RED;
                t = 0;
                apply_state(state);
            }
            break;

        case STATE_RED:
            // If a pedestrian is waiting, immediately start the dedicated ped red timer
            if (ped_requested) {
                state = STATE_PED_RED;
                t = 0;
                apply_state(state);
                ped_requested = 0; // consume request
                printf("[PED] Crossing started (%d s).\n", PED_CROSS_DURATION);
                break;
            }

            if (t >= RED_DURATION) {
                state = STATE_GREEN;
                t = 0;
                apply_state(state);
            }
            break;

        case STATE_PED_RED:
            if (t >= PED_CROSS_DURATION) {
                state = STATE_GREEN;
                t = 0;
                apply_state(state);
                printf("[PED] Crossing ended. Back to GREEN.\n");
            }
            break;
    }

    // optional status line (nice for judging)
    const char *name =
        (state == STATE_GREEN)   ? "GREEN" :
        (state == STATE_YELLOW)  ? "YELLOW" :
        (state == STATE_RED)     ? "RED" : "PED_RED";

    printf("State=%s  t=%d\n", name, t);
}
