#include <stdio.h>
#include "traffic.light.h"
#include <wiringPi.h>  // Add wiringPi for GPIO

static int timer = 0;
static enum TrafficLight current_state = Red;
static unsigned int pedestrian_request = 0;
static unsigned int emergency_vehicle_detected = 0;

// Initialize GPIO pins
void init_gpio(void) {
    if (wiringPiSetup() == -1) {
        printf("ERROR: Failed to initialize wiringPi\n");
        return;
    }
    
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(PED_PIN, OUTPUT);
    
    // Turn all LEDs off initially
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(PED_PIN, LOW);
    
    printf("GPIO initialized successfully\n");
}

// Set hardware LEDs based on traffic light state
void set_traffic_light(enum TrafficLight state) {
    // First turn all LEDs off
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(PED_PIN, LOW);
    
    switch(state) {
        case Red:
            digitalWrite(RED_PIN, HIGH);
            break;
        case Yellow:
            digitalWrite(YELLOW_PIN, HIGH);
            break;
        case Green:
            digitalWrite(GREEN_PIN, HIGH);
            break;
        case PedestrianCrossing:
            digitalWrite(RED_PIN, HIGH);        // Red light for cars
            digitalWrite(PED_PIN, HIGH);        // Green for pedestrians
            break;
        case blinking_yellow:
            // Blinking effect will be handled in update_traffic_light
            digitalWrite(YELLOW_PIN, HIGH);
            break;
    }
}

// Cleanup GPIO on exit
void cleanup_gpio(void) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(PED_PIN, LOW);
    printf("GPIO cleanup completed\n");
}

int get_current_state(void) {
    return current_state;
}

void request_pedestrian_crossing(void) {
    pedestrian_request = 1;
    printf("Pedestrian crossing requested\n");
}

void detect_emergency_vehicle(void) {
    emergency_vehicle_detected = 1;
    printf("Emergency vehicle detected\n");
}

enum TrafficLight getNextState(enum TrafficLight current) {
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
    
    // Handle blinking yellow state
    if (current_state == blinking_yellow) {
        // Toggle yellow LED every second for blinking effect
        if (timer % 2 == 0) {
            digitalWrite(YELLOW_PIN, HIGH);
        } else {
            digitalWrite(YELLOW_PIN, LOW);
        }
    }
    
    if (emergency_vehicle_detected) {
        current_state = blinking_yellow;
        emergency_vehicle_detected = 0;
        timer = 0;
        set_traffic_light(current_state);
        return;
    }
    
    if (pedestrian_request && current_state == Red) {
        current_state = PedestrianCrossing;
        pedestrian_request = 0;
        timer = 0;
        set_traffic_light(current_state);
        return;
    }
    
    switch (current_state) {
        case Green:
            if (timer >= GREEN_DURATION) {
                current_state = Yellow;
                timer = 0;
                set_traffic_light(current_state);
            }
            break;
        case Yellow:
            if (timer >= YELLOW_DURATION) {
                current_state = Red;
                timer = 0;
                set_traffic_light(current_state);
            }
            break;
        case Red:
            if (timer >= RED_DURATION) {
                current_state = Green;
                timer = 0;
                set_traffic_light(current_state);
            }
            break;
        case PedestrianCrossing:
            if (timer >= PEDESTRIAN_CROSSING_DURATION) {
                current_state = Green;
                timer = 0;
                set_traffic_light(current_state);
            }
            break;
        case blinking_yellow:
            if (timer >= BLINKING_YELLOW_DURATION) {
                current_state = Red;
                timer = 0;
                set_traffic_light(current_state);
            }
            break;
    }
}