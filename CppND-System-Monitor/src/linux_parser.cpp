#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

template <class T> T getValueFromFile(const string file_path, const string variable) {
  string line;
  string key;
  T result;
  std::ifstream filestream(file_path);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == variable) {
          linestream >> result;
          return result;

        }
      }
    } 
  }
  return result;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string file_path = kProcDirectory + kMeminfoFilename;
  
  const float mem_total = getValueFromFile<float> (file_path, "MemTotal:");
  const float mem_free = getValueFromFile<float> (file_path, "MemFree:");

  const float mem_util = (mem_total - mem_free) / mem_total;
  return mem_util;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() { 
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

// Get a vector of long jiffies
vector<long> LinuxParser::getJiffies() {
  vector<string> jiffies_str = CpuUtilization();
  vector<long> jiffies_long(jiffies_str.size());
  std::transform(jiffies_str.begin(), jiffies_str.end(), jiffies_long.begin(), [](const string& val)
  {
    return stol(val);
  });
  return jiffies_long;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return (ActiveJiffies() + IdleJiffies());
}

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  string file_name = kProcDirectory + std::to_string(pid) + kStatFilename;
  int index = 13;
  string line;
  string val;
  string utime, stime, cutime, cstime;
  long result;
  std::ifstream stream(file_name);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for(int i = 0; i < index; i++) {
      linestream >> val;
    }
    linestream >> utime >> stime >> cutime >> cstime;
    result = std::stol(utime) + std::stol(stime) + std::stol(cutime) + std::stol(cstime);
  }
  return result/sysconf(_SC_CLK_TCK); 
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  vector<long> jiffies = getJiffies();
  vector<CPUStates> non_idle = {kUser_, kNice_, kSystem_, kIRQ_, kSoftIRQ_, kSteal_};
  long total = 0;
  for(CPUStates state: non_idle) {
    total += jiffies[state];
  }
  return total;
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {   
  vector<long> jiffies = getJiffies();
  vector<CPUStates> idle = {kIdle_, kIOwait_};
  long total = 0;
  for(CPUStates state: idle) {
    total += jiffies[state];
  }
  return total; 
}

// DONE: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  vector<string> jiffies;
  string line;
  string index;
  string val;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> index;
    if (index == "cpu") {
      for(int i = 0; i < 10; i++) {
        linestream >> val;
        jiffies.emplace_back(val);
      }
    }
  }
  return jiffies;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string file_path = kProcDirectory + kStatFilename;

  const int total_processes = getValueFromFile<int> (file_path, "processes");

  return total_processes;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string file_path = kProcDirectory + kStatFilename;

  const int running_processes = getValueFromFile<int> (file_path, "procs_running");

  return running_processes;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string file_name = kProcDirectory + std::to_string(pid) + kCmdlineFilename;
  string line;
  std::ifstream filestream(file_name);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      string command = linestream.str();
      return command;
    }
  }
  return line;
}

// DONE: Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string file_path = kProcDirectory + std::to_string(pid) + kStatusFilename;

  const long memory = getValueFromFile<long> (file_path, "VmSize:");

  return std::to_string(memory);
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string file_path = kProcDirectory + std::to_string(pid) + kStatusFilename;

  const int uid = getValueFromFile<int> (file_path, "Uid:");

  return std::to_string(uid);
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string line;
  string user;
  string uid = Uid(pid);
  string scanned_id;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), 'x', ' ');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> scanned_id) {
        if (scanned_id == uid) {
          return user;
        }
      }
    }
  }
  return user;
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  string file_name = kProcDirectory + std::to_string(pid) + kStatFilename;
  int index = 22;
  string line;
  string val;
  long time;
  std::ifstream stream(file_name);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for(int i = 0; i < index; i++) {
      linestream >> val;
    }
    time = std::stol(val)/sysconf(_SC_CLK_TCK);
  }
  return time;
}
