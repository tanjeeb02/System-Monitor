#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) { 
    
    long hours {0}, minutes {0};
  	string s_hr, s_min, s_sec, time;
  
    hours  = seconds / 3600;
    seconds = seconds % 3600;
    minutes = seconds / 60;
    seconds = seconds % 60;
  
  	s_hr = std::to_string(hours);
  	s_min = std::to_string(minutes);
    s_sec = std::to_string(seconds);
  
  	s_hr.insert(0, 2 - s_hr.length(), '0');
  	s_min.insert(0, 2 - s_min.length(), '0');
  	s_sec.insert(0, 2 - s_sec.length(), '0');
  
    time = s_hr + ":" + s_min + ":" + s_sec;

    return time; 
    
}