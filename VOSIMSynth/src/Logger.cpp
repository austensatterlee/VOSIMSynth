#include "Logger.h"

synui::Logger& synui::Logger::instance()
{
    static Logger singleton;
    return singleton;
}

void synui::Logger::log(const std::string& a_channel, const std::string& a_msg)
{
    if (m_logs.size() < m_maxLogs - 1)
    {
        m_logs.push_back({a_channel, a_msg});
        m_nextIndex++;
    }
    else
    {
        m_logs[m_nextIndex] = {a_channel, a_msg};
        m_nextIndex = m_nextIndex % m_maxLogs;
    }

    for (auto& listener : m_listeners)
        listener();
}

const std::vector<std::pair<std::string, std::string>>& synui::Logger::getLogs() const
{
    return m_logs;
}

void synui::Logger::addListener(Listener a_listener)
{
    m_listeners.push_back(a_listener);
}

void synui::Logger::reset()
{
    m_listeners.clear();
    m_logs.clear();
    m_nextIndex = 0;
}

synui::Logger::Logger(): m_maxLogs(1000), m_nextIndex(0) {}
