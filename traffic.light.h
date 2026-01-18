#ifndef TRAFFIC_LIGHT_H 
#define TRAFFIC_LIGHT_H 

// GPIO pin definitions
#define RED_PIN         17    // GPIO 17
#define YELLOW_PIN      27    // GPIO 27
#define GREEN_PIN       22    // GPIO 22
#define PED_BUTTON_PIN  23    // GPIO 23 (Pedestrian request button)

// Traffic light states
typedef enum TrafficLight { 
    Red, 
    Yellow, 
    Green, 
    PedestrianCrossing,  // Pedestrians can cross
    BlinkingRed          // For emergency vehicles only
} TrafficLight;

// Timing constants
#define GREEN_DURATION          10
#define YELLOW_DURATION         10 
#define RED_DURATION            10 
#define PEDESTRIAN_CROSSING_DURATION 8
#define BLINKING_RED_DURATION   5

// Accelerated timings (when button is pressed)
#define ACCELERATED_GREEN_DURATION   2
#define ACCELERATED_YELLOW_DURATION  2
#define ACCELERATED_RED_DURATION     2

// Button debounce time in milliseconds
#define BUTTON_DEBOUNCE_MS 50

// Function prototypes
void update_traffic_light(void);
TrafficLight get_current_state(void);
void request_pedestrian_crossing(void);
void detect_emergency_vehicle(void);
TrafficLight getNextState(TrafficLight current_state);
void check_pedestrian_button(void);

// GPIO functions
void init_gpio(void);
void set_traffic_light(TrafficLight state);
void cleanup_gpio(void);

// External variables (for main.c access)
extern int timer;
extern int acceleration_mode;
extern int pedestrian_requested;

#endif // TRAFFIC_LIGHT_H