#include "processor.h"
#include "linux_parser.h"

Processor::Processor() {
    prev_idle_ = LinuxParser::IdleJiffies();
    prev_active_ = LinuxParser::ActiveJiffies();
    prev_total_ = prev_idle_ + prev_active_;
}

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() { 
    long cur_idle = LinuxParser::IdleJiffies();
    long cur_active = LinuxParser::ActiveJiffies();
    long cur_total = cur_idle + cur_active;

    long total_d = cur_total - prev_total_;
    long idle_d = cur_idle - prev_idle_;

    float percentage = 1.0*(total_d - idle_d)/total_d;
    return percentage;
}