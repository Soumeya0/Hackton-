#include <stdio.h>
#include "traffic.light.h"

static int timer = 0;
static enum TrafficLight current_state = Red;
int get_current_state() {
    return current_state;
}
static unsigned int pedestrian_request = 0;
static unsigned int emergency_vehicle_detected = 0; // Declare emergency_vehicle_detected

void request_pedestrian_crossing(void) {
    pedestrian_request = 1;
    printf("Pedestrian crossing requested\n");
}
void detect_emergency_vehicle(void) {
    emergency_vehicle_detected = 1;
    printf("Emergency vehicle detected\n");
}
enum TrafficLight getNextState (enum TrafficLight current) {
    switch (current) {
        case Red:
            return Green;
        case Green:
            return Yellow;
        case Yellow:
            return Red;
        case PedestrianCrossing:
            return Green;
        case blinking_yellow:
            return Green;
        default:
            return Red;
    }
}

void update_traffic_light(void) {
    timer++;
    printf("Timer:%d, State:%d\n", timer, current_state);
    if (emergency_vehicle_detected) {
        current_state = blinking_yellow;
        emergency_vehicle_detected = 0;
        timer = 0;
        return;
    }
        if (pedestrian_request && current_state == Red) {
        current_state = PedestrianCrossing;
        pedestrian_request = 0;
        timer = 0;
        return;
    }
    switch (current_state) {
        case Green:
            if (timer >= GREEN_DURATION) {
                current_state = Yellow;
                timer = 0;
            }
            break;
        case Yellow:
            if (timer >= YELLOW_DURATION) {
                current_state = Red;
                timer = 0;
            }
            break;
        case Red:
            if (timer >= RED_DURATION) {
                current_state = Green;
                timer = 0;
            }
            break;
        case PedestrianCrossing:
            if (timer >= PEDESTRIAN_CROSSING_DURATION) {
                current_state = Green;
                timer = 0;
            }
            break;
        case blinking_yellow:
            if (timer >= BLINKING_YELLOW_DURATION) {
                current_state = Red;
                timer = 0;
            }
            break;
    }
}


