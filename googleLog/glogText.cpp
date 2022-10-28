#include "log.h"

/////////////////////////////////////////////////////////////
int main()
{
    std::string str("Test Application");
    initLog(str);

    WRITE_INFOLOG(__FUNCTION__);
    WRITE_INFOLOG(__LINE__);

    WRITE_INFOLOG("aaa<<32k<<fjkda<<32");
    
    WRITE_WARNLOG("WRITE_WARNLOG initial opengl 初始化OpenGL");

    int i = 999;
    WRITE_WARNLOG(i);

    WRITE_WARNLOG(__FILE__); 

    WRITE_ERRORLOG("WRITE_ERRORLOG initial opengl 初始化OpenGL");

    std::string strError("2 WRITE_ERRORLOG initial opengl 初始化OpenGL");
    WRITE_ERRORLOG(strError.c_str()); 


    closeLog();

    return 0;
}

