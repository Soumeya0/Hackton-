#include <stdio.h>
#include <time.h>
#include "traffic_light.h"
#include <wiringPi.h>   

// Define variables (also declared in header)
int timer = 0;
int acceleration_mode = 0;
int pedestrian_requested = 0;

static TrafficLight current_state = Red;
static unsigned int pedestrian_active = 0;     // Currently in pedestrian mode
static unsigned int emergency_vehicle_detected = 0;
static unsigned long last_button_press = 0;

// Initialize GPIO pins
void init_gpio(void) {
    if (wiringPiSetupGpio() == -1) {
        printf("ERROR: Failed to initialize wiringPi\n");
        return;
    }
    
    // Set LED pins as OUTPUT
    pinMode(RED_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    
    // Set pedestrian button as INPUT with pull-up resistor
    pinMode(PED_BUTTON_PIN, INPUT);
    pullUpDnControl(PED_BUTTON_PIN, PUD_UP);  // Button connects to GND when pressed
    
    // Turn all LEDs off initially
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    
    printf("GPIO initialized successfully\n");
    printf("LEDs: RED=GPIO%d, YELLOW=GPIO%d, GREEN=GPIO%d\n", 
           RED_PIN, YELLOW_PIN, GREEN_PIN);
    printf("Button: PEDESTRIAN=GPIO%d (Active LOW)\n", PED_BUTTON_PIN);
}

// Set hardware LEDs based on traffic light state
void set_traffic_light(TrafficLight state) {
    switch(state) {
        case Red:
            digitalWrite(RED_PIN, HIGH);
            digitalWrite(YELLOW_PIN, LOW);
            digitalWrite(GREEN_PIN, LOW);
            break;
            
        case Yellow:
            digitalWrite(RED_PIN, LOW);
            digitalWrite(YELLOW_PIN, HIGH);
            digitalWrite(GREEN_PIN, LOW);
            break;
            
        case Green:
            digitalWrite(RED_PIN, LOW);
            digitalWrite(YELLOW_PIN, LOW);
            digitalWrite(GREEN_PIN, HIGH);
            break;
            
        case PedestrianCrossing:
            // Red for cars, pedestrians can cross
            digitalWrite(RED_PIN, HIGH);     // Cars stop
            digitalWrite(YELLOW_PIN, LOW);
            digitalWrite(GREEN_PIN, LOW);
            break;
            
        case BlinkingRed:
            // Initial state for blinking - will be handled in update
            digitalWrite(RED_PIN, HIGH);
            digitalWrite(YELLOW_PIN, LOW);
            digitalWrite(GREEN_PIN, LOW);
            break;
    }
}

// Check pedestrian button with debouncing
void check_pedestrian_button(void) {
    static int last_button_state = HIGH;
    int button_state = digitalRead(PED_BUTTON_PIN);
    unsigned long current_time = millis();
    
    // Check for button press (LOW because of pull-up)
    if (button_state == LOW && last_button_state == HIGH) {
        // Debounce check
        if (current_time - last_button_press > BUTTON_DEBOUNCE_MS) {
            if (!pedestrian_requested && current_state != PedestrianCrossing) {
                pedestrian_requested = 1;
                printf("Button pressed! Accelerating to RED for pedestrian crossing\n");
                
                // Enter acceleration mode
                acceleration_mode = 1;
                
                // If we're in Green or Yellow, speed up the transition
                if (current_state == Green) {
                    printf("   Speeding up GREEN phase\n");
                } else if (current_state == Yellow) {
                    printf("   Speeding up YELLOW phase\n");
                } else if (current_state == Red) {
                    printf("   Already at RED, will start pedestrian crossing soon\n");
                }
            }
            last_button_press = current_time;
        }
    }
    
    last_button_state = button_state;
}

// Cleanup GPIO on exit
void cleanup_gpio(void) {
    digitalWrite(RED_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    printf("GPIO cleanup completed\n");
}

TrafficLight get_current_state(void) {
    return current_state;
}

void request_pedestrian_crossing(void) {
    if (!pedestrian_requested && current_state != PedestrianCrossing) {
        pedestrian_requested = 1;
        acceleration_mode = 1;
        printf("Keyboard request! Accelerating to RED for pedestrian crossing\n");
    }
}

void detect_emergency_vehicle(void) {
    emergency_vehicle_detected = 1;
    printf("Emergency vehicle detected!\n");
}

TrafficLight getNextState(TrafficLight current) {
    switch (current) {
        case Red:
            return Green;
        case Green:
            return Yellow;
        case Yellow:
            return Red;
        case PedestrianCrossing:
            return Green;  // After pedestrian crossing, go back to normal
        case BlinkingRed:
            return Red;    // After emergency, return to normal Red
        default:
            return Red;
    }
}

void update_traffic_light(void) {
    timer++;
    
    // Check physical button
    check_pedestrian_button();
    
    printf("Timer:%d, State:%d, Acc:%d, Req:%d\n", 
           timer, current_state, acceleration_mode, pedestrian_requested);
    
    // Handle emergency vehicle override (can interrupt any state)
    if (emergency_vehicle_detected) {
        printf("EMERGENCY: Switching to Blinking Red mode\n");
        current_state = BlinkingRed;
        emergency_vehicle_detected = 0;
        pedestrian_requested = 0;
        acceleration_mode = 0;
        timer = 0;
        set_traffic_light(current_state);
        return;
    }
    
    // Handle blinking red state for emergency
    if (current_state == BlinkingRed) {
        // Fast blinking red (2Hz) for emergency
        if (timer % 1 == 0) {
            static int blink_toggle = 0;
            blink_toggle = !blink_toggle;
            digitalWrite(RED_PIN, blink_toggle ? HIGH : LOW);
            digitalWrite(YELLOW_PIN, LOW);
            digitalWrite(GREEN_PIN, LOW);
        }
        
        if (timer >= BLINKING_RED_DURATION) {
            current_state = Red;
            timer = 0;
            set_traffic_light(current_state);
            printf("Emergency mode ended, back to normal Red\n");
        }
        return;
    }
    
    // Handle pedestrian crossing state
    if (current_state == PedestrianCrossing) {
        if (timer >= PEDESTRIAN_CROSSING_DURATION) {
            current_state = Green;
            pedestrian_active = 0;
            acceleration_mode = 0;
            timer = 0;
            set_traffic_light(current_state);
            printf(" Pedestrian crossing ended, switching to Green\n");
        }
        return;
    }
    
    // Check if we should start pedestrian crossing (must be at Red)
    if (pedestrian_requested && current_state == Red) {
        printf(" Starting pedestrian crossing phase\n");
        current_state = PedestrianCrossing;
        pedestrian_requested = 0;
        pedestrian_active = 1;
        acceleration_mode = 0;
        timer = 0;
        set_traffic_light(current_state);
        return;
    }
    
    // Determine current duration based on acceleration mode
    int current_green_duration = acceleration_mode ? ACCELERATED_GREEN_DURATION : GREEN_DURATION;
    int current_yellow_duration = acceleration_mode ? ACCELERATED_YELLOW_DURATION : YELLOW_DURATION;
    int current_red_duration = acceleration_mode ? ACCELERATED_RED_DURATION : RED_DURATION;
    
    // Handle normal/accelerated state transitions
    switch (current_state) {
        case Green:
            if (timer >= current_green_duration) {
                current_state = Yellow;
                timer = 0;
                set_traffic_light(current_state);
                if (acceleration_mode) {
                    printf("Accelerated: Switching from Green to Yellow\n");
                } else {
                    printf("Switching from Green to Yellow\n");
                }
            }
            break;
            
        case Yellow:
            if (timer >= current_yellow_duration) {
                current_state = Red;
                timer = 0;
                set_traffic_light(current_state);
                if (acceleration_mode) {
                    printf(" Accelerated: Switching from Yellow to Red\n");
                } else {
                    printf(" Switching from Yellow to Red\n");
                }
            }
            break;
            
        case Red:
            if (!pedestrian_requested && timer >= current_red_duration) {
                // Only proceed if no pedestrian waiting
                current_state = Green;
                timer = 0;
                acceleration_mode = 0;  // Reset acceleration
                set_traffic_light(current_state);
                printf("Switching from Red to Green\n");
            }
            // If pedestrian_requested is true, we'll catch it at the next cycle
            break;
    }
}
