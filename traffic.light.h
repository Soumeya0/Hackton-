#ifndef TRAFFIC_LIGHT_H 
#define TRAFFIC_LIGHT_H 

// Add GPIO pin definitions
#define RED_PIN     0     // GPIO 17 (wiringPi pin 0)
#define YELLOW_PIN  1     // GPIO 18 (wiringPi pin 1)
#define GREEN_PIN   2     // GPIO 27 (wiringPi pin 2)
#define PED_PIN     3     // GPIO 22 (wiringPi pin 3) - Pedestrian signal

enum TrafficLight { 
    Red, 
    Yellow, 
    Green, 
    PedestrianCrossing, 
    blinking_yellow 
}; 

#define GREEN_DURATION 10
#define YELLOW_DURATION 10 
#define RED_DURATION 10 
#define BLINKING_YELLOW_DURATION 5
#define PEDESTRIAN_CROSSING_DURATION 8

// Function prototypes
void update_traffic_light(void);
int get_current_state(void);
void request_pedestrian_crossing(void);
void detect_emergency_vehicle(void);
enum TrafficLight getNextState(enum TrafficLight current_state);

// GPIO functions
void init_gpio(void);
void set_traffic_light(enum TrafficLight state);
void cleanup_gpio(void);

#endif // TRAFFIC_LIGHT_H