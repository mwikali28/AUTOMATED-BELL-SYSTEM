// Member 3: Scheduling Logic
#include "scheduler.h"

BellEvent schedule[] = {
    {8, 0, "Warning", false},
    {8, 5, "Start", false},
    // ... more events
};

bool checkSchedule(int currentHour, int currentMinute, 
                   int currentSecond, int& lastRung) {
    for (int i = 0; i < scheduleSize; i++) {
        if (currentHour == schedule[i].hour && 
            currentMinute == schedule[i].minute && 
            currentSecond < 3 && lastRung != i) {
            lastRung = i;
            return true;
        }
    }
    return false;
}
