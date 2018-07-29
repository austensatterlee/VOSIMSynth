#include "vosimlib/Logging.h"

syn::TimingTracer::~TimingTracer() {
    auto elapsed = std::chrono::high_resolution_clock::now() - m_startTime;
    std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    GlobalLogFile::instance().log(m_funcName, m_line, "Finished in ", us.count(), " us.");
}

void syn::TimingTracer::_recordStartTime() {
    m_startTime = std::chrono::high_resolution_clock::now();
}