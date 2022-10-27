#include <string>

class baseinfoObj
{
public:
    baseinfoObj() = default;
    ~baseinfoObj() = default;

public:
    std::string m_strVer;
    std::string m_strAuthor;
    std::string m_strCreateTime;
    std::string m_strEncrypt;
    std::string m_strMask;
};