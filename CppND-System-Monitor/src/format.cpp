#include <iomanip>
#include <sstream>
#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::FormProper(int num) {
  std::stringstream ss;
  ss << std::setw(2) << std::setfill('0') << num;
  return ss.str();
}

string Format::ElapsedTime(long seconds) {
  int elapsed_hours = seconds / 3600;
  int elapsed_minutes = (seconds % 3600) / 60;
  int elapsed_seconds = seconds % 60;

  string elapsed_time = FormProper(elapsed_hours) + ":" +
                        FormProper(elapsed_minutes) + ":" +
                        FormProper(elapsed_seconds);
  return elapsed_time;
}