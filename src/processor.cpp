#include "processor.h"
#include "linux_parser.h"
#include <thread>

float Processor::Utilization() {
    int numCpus = std::thread::hardware_concurrency();
    std::string line, uptime, idletime;
    std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kUptimeFilename);
    if (stream.is_open()) {
        std::getline(stream, line);
        std::istringstream linestream(line);
        linestream >> uptime >> idletime;
    }
    float CpuUsage = (numCpus * std::stol(uptime) - std::stol(idletime));

    return CpuUsage/(numCpus * std::stol(uptime));
}