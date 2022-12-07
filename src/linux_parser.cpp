#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>


using std::stof;
using std::string;
using std::to_string;
using std::vector;


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


string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}


vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    if (file->d_type == DT_DIR) {
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


float LinuxParser::MemoryUtilization() {
  float total, free, mem;
  string key, val, line;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> val) {
        if (key == "MemTotal:") {
          total = std::stof(val);
        } else if (key == "MemFree:") {
          free = std::stof(val);
        }
      }
    }
  }

  mem = (total - free) / total;
  return mem;
}


long LinuxParser::UpTime() {
  long uptime;
  string upstr, line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> upstr;
  }

  uptime = stol(upstr);
  return uptime;
}


long LinuxParser::TotalJiffies() {
  long Total{0};
  Total = LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
  return Total;
}


long LinuxParser::ActiveJiffies(int pid) {
  long total_time;
  string val, line;
  vector<string> vec;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> val) {
      vec.push_back(val);
    }
  }
  long utime{0}, stime{0}, cutime{0}, cstime{0};
  utime = std::stol(vec[13]);
  stime = std::stol(vec[14]);
  cutime = std::stol(vec[15]);
  cstime = std::stol(vec[16]);

  total_time = utime + stime + cutime + cstime;
  return total_time / sysconf(_SC_CLK_TCK);
}


long LinuxParser::ActiveJiffies() {
  vector<string> jiffies = LinuxParser::CpuUtilization();
  long NonIdle{0}, sum_1{0}, sum_2{0};
  for (auto i{0}; i < 3; i++) {
    sum_1 += stol(jiffies[i]);
  }
  for (auto i{5}; i < 8; i++) {
    sum_2 += stol(jiffies[i]);
  }
  NonIdle = sum_1 + sum_2;
  return NonIdle;
}


long LinuxParser::IdleJiffies() {
  vector<string> jiffies = LinuxParser::CpuUtilization();
  long Idle{0};
  for (auto i{3}; i < 5; i++) {
    Idle += stol(jiffies[i]);
  }
  return Idle;
}


vector<string> LinuxParser::CpuUtilization() {
  string key, value, line;
  vector<string> jiffies;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> key;
    while (linestream >> value) {
      jiffies.push_back(value);
    }
  }
  return jiffies;
}

float LinuxParser::CpuUtilization(int pid) {
  long total_time = LinuxParser::ActiveJiffies(pid);
  long seconds = LinuxParser::UpTime() - LinuxParser::UpTime(pid);
  float util = 100 * (total_time / seconds);
  return util;
}


int LinuxParser::TotalProcesses() {
  int value{0};
  string key, line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "processes") {
        linestream >> value;
      }
    }
  }
  return value;
}


int LinuxParser::RunningProcesses() {
  int value{0};
  string key, line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "procs_running") {
        linestream >> value;
      }
    }
  }
  return value;
}


string LinuxParser::Command(int pid) {
  string cmd;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, cmd);
  }
  return cmd;
}


string LinuxParser::Ram(int pid) {
  string key, ram, line;
  long value{0};
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> value;
        value /= 1000;
        ram = std::to_string(value);
      }
    }
  }
  return ram;
}


string LinuxParser::Uid(int pid) {
  string key, value, uid, line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "Uid:") {
        uid = value;
      }
    }
  }
  return uid;
}


string LinuxParser::User(int pid) {
  string user, x, temp, uid, line;
  string userUid = LinuxParser::Uid(pid);
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> temp >> x >> uid;
      if (uid == userUid) {
        user = temp;
        break;
      }
    }
    return user;
  }
  return nullptr;
}


long LinuxParser::UpTime(int pid) {
  string val, line;
  vector<string> vec;
  long uptime{0};
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    while (linestream >> val) {
      vec.push_back(val);
    }
  }
  uptime = stol(vec[21]) / sysconf(_SC_CLK_TCK);
  return uptime;
}