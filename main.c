#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "traffic.light.h"

// Signal handler for cleanup
void handle_signal(int sig) {
    printf("\nReceived signal %d, cleaning up...\n", sig);
    cleanup_gpio();
    exit(0);
}

void printStateInfo(TrafficLight state) {
    switch(state) {
        case Red:
            printf("🔴 Current: RED ");
            if (pedestrian_requested) {
                printf("(Pedestrian waiting - will cross soon)\n");
            } else {
                printf("(Switch to GREEN in %d seconds)\n", RED_DURATION - timer);
            }
            break;
        case Yellow:
            printf("🟡 Current: YELLOW ");
            if (acceleration_mode) {
                printf("⏩ ACCELERATED ");
            }
            printf("(Switch to RED in %d seconds)\n", YELLOW_DURATION - timer);
            break;
        case Green:
            printf("🟢 Current: GREEN ");
            if (acceleration_mode) {
                printf("⏩ ACCELERATED ");
            }
            printf("(Switch to YELLOW in %d seconds)\n", GREEN_DURATION - timer);
            break;
        case PedestrianCrossing:
            printf("🚶 Current: PEDESTRIAN CROSSING ");
            printf("(Ends in %d seconds)\n", PEDESTRIAN_CROSSING_DURATION - timer);
            break;
        case BlinkingRed:
            printf("🚑 Current: EMERGENCY VEHICLE ");
            printf("(Blinking Red - Ends in %d seconds)\n", BLINKING_RED_DURATION - timer);
            break;
    }
}

int main() {
    // Set up signal handling for clean exit
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    printf("================================\n");
    printf("   TRAFFIC LIGHT SIMULATION\n");
    printf("================================\n\n");
    
    printf("Initializing GPIO...\n");
    init_gpio();
    
    printf("\n=== HARDWARE CONFIGURATION ===\n");
    printf("Traffic Lights:\n");
    printf("  🔴 RED:     GPIO %d\n", RED_PIN);
    printf("  🟡 YELLOW:  GPIO %d\n", YELLOW_PIN);
    printf("  🟢 GREEN:   GPIO %d\n", GREEN_PIN);
    printf("Pedestrian Button: GPIO %d (Pull-up, active LOW)\n\n", PED_BUTTON_PIN);
    
    printf("=== TIMING CONFIGURATION ===\n");
    printf("Normal Cycle:\n");
    printf("  GREEN:  %d seconds\n", GREEN_DURATION);
    printf("  YELLOW: %d seconds\n", YELLOW_DURATION);
    printf("  RED:    %d seconds\n", RED_DURATION);
    
    printf("\nAccelerated Cycle (when button pressed):\n");
    printf("  GREEN:  %d seconds\n", ACCELERATED_GREEN_DURATION);
    printf("  YELLOW: %d seconds\n", ACCELERATED_YELLOW_DURATION);
    printf("  RED:    %d seconds\n", ACCELERATED_RED_DURATION);
    
    printf("\nSpecial Modes:\n");
    printf("  Pedestrian Crossing: %d seconds\n", PEDESTRIAN_CROSSING_DURATION);
    printf("  Emergency Vehicle:   %d seconds (Blinking Red)\n\n", BLINKING_RED_DURATION);
    
    printf("=== HOW IT WORKS ===\n");
    printf("1. Press button (GPIO %d) or 'p' key\n", PED_BUTTON_PIN);
    printf("2. Current light phase ACCELERATES to reach RED faster\n");
    printf("3. At RED light, pedestrian crossing begins\n");
    printf("4. Pedestrians see RED light (cars stopped) for %d seconds\n", PEDESTRIAN_CROSSING_DURATION);
    printf("5. Press 'e' anytime for emergency vehicle mode\n");
    printf("================================\n\n");
    
    // Clear input buffer
    int c;
    while((c = getchar()) != EOF && c != '\n');
    
    int debug_mode = 0;
    TrafficLight last_state = Red;
    TrafficLight current_state = get_current_state();
    
    // Set initial state
    set_traffic_light(current_state);
    
    while(1) {
        update_traffic_light();
        TrafficLight state = get_current_state();
        
        last_state = state;
        
        // Show state information
        if (!debug_mode) {
            printStateInfo(state);
        }
        
        printf("\n[Button:GPIO%d] Options: p=pedestrian, e=emergency, q=quit, Enter=next\n", PED_BUTTON_PIN);
        printf("> ");
        
        char input;
        if (scanf("%c", &input) != EOF) {
            if (input == 'p') {
                request_pedestrian_crossing();
            } else if (input == 'e') {
                detect_emergency_vehicle();
                printf("⚠️  Emergency vehicle detected!\n");
            } else if (input == 'q') {
                printf("Exiting simulation...\n");
                cleanup_gpio();
                break;
            }
        }
        
        // Clear input buffer
        while ((c = getchar()) != EOF && c != '\n');
        
        sleep(1);
        printf("\n---\n");
    }
    
    return 0;
}