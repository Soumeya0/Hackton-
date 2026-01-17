#ifndef TRAFFIC_LIGHT_H 
#define TRAFFIC_LIGHT_H 
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
void update_traffic_light(void);
int get_current_state(void);
void request_pedestrian_crossing(void);
void detect_emergency_vehicle(void);
enum TrafficLight getNextState( enum TrafficLight current_state);
#endif // TRAFFIC_LIGHT_H