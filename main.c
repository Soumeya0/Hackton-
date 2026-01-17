#include <stdio.h>
#include <unistd.h>
#include "traffic.light.h"

void printStateInfo(enum TrafficLight state) {
    switch(state) {
        case Red:
            printf("🚦 Current: RED (Should switch to GREEN after %d seconds)\n", RED_DURATION);
            break;
        case Yellow:
            printf("🚦 Current: YELLOW (Should switch to RED after %d seconds)\n", YELLOW_DURATION);
            break;
        case Green:
            printf("🚦 Current: GREEN (Should switch to YELLOW after %d seconds)\n", GREEN_DURATION);
            break;
        case PedestrianCrossing:
            printf("🚶 Current: PEDESTRIAN CROSSING (%d seconds)\n", PEDESTRIAN_CROSSING_DURATION);
            break;
        case blinking_yellow:
            printf("🚦 Current: BLINKING YELLOW (%d seconds)\n", BLINKING_YELLOW_DURATION);
            break;
    }
}

int main() {
    printf("================================\n");
    printf("   TRAFFIC LIGHT SIMULATION\n");
    printf("================================\n\n");
    
    printf("Durations: RED=%ds, YELLOW=%ds, GREEN=%ds\n", 
           RED_DURATION, YELLOW_DURATION, GREEN_DURATION);
    printf("Special: Pedestrian=%ds, Blink modes=%ds\n\n",
           PEDESTRIAN_CROSSING_DURATION, BLINKING_YELLOW_DURATION);
    
    // Clear input buffer
    int c;
    while((c = getchar()) != EOF && c != '\n');
    
    int debug_mode = 0;  // 0 = normal, 1 = debug
    int consecutive_reds = 0;
    enum TrafficLight last_state = Red;
    
    while(1) {
        update_traffic_light();
        enum TrafficLight state = get_current_state();
        
        // Check if stuck in RED
        if (state == Red) {
            consecutive_reds++;
            if (consecutive_reds > RED_DURATION + 5) {  // Allow some buffer
                printf("\n⚠️  WARNING: Might be stuck in RED state!\n");
                printf("   Last state was: %d, Timer might not be resetting\n", last_state);
            }
        } else {
            consecutive_reds = 0;
        }
        
        last_state = state;
        
        // Show state information
        if (!debug_mode) {
            printStateInfo(state);
        }
        
        printf("\nOptions: p=pedestrian, e=emergency q=quit, Enter=next\n");
        printf("> ");
        
        char input;
        if (scanf("%c", &input) != EOF) {
            if (input == 'p') {
                request_pedestrian_crossing();
                printf("✓ Pedestrian crossing requested (will activate at next RED)\n");
            } else if (input == 'e') {
                detect_emergency_vehicle();
                printf("⚠️ Emergency vehicle detected! Overriding normal cycle.\n");
            } else if (input == 'q') {
                printf("Exiting simulation...\n");
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