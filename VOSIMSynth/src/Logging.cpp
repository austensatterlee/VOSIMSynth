#include "Logging.h"
#include <sstream>

synui::TimingTracer::~TimingTracer() {
    auto elapsed = std::chrono::high_resolution_clock::now() - m_startTime;
    std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    std::ostringstream ss; 
    ss << "Finished in " << us.count() << " us.";
    Trace(m_funcName.c_str(), m_line, ss.str().c_str());
}

void synui::TimingTracer::_recordStartTime() {
    m_startTime = std::chrono::high_resolution_clock::now();
}