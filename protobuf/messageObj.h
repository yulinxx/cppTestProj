#include <string>

class messageObj
{
public:
    messageObj() = default;
    ~messageObj() = default;
public:
    int m_nDataTime = 0;
    std::string m_strHostName;
    std::string m_strIP;
    std::string m_strInfo;
};