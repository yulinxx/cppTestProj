#ifndef LIBEXPORTS_H
#define LIBEXPORTS_H

// #include <qglobal.h>

#ifdef _WIN32
    #ifdef EXPORT_LIB
        #define EXPORT_API __declspec(dllexport)
    #else
        #define EXPORT_API __declspec(dllimport)
    #endif
#else
    #define EXPORT_API
#endif

//  #ifdef _WIN32
//      #ifdef EXPORT_LIB
//          #define EXPORT_API Q_DECL_EXPORT
//      #else
//          #define EXPORT_API Q_DECL_IMPORT
//      #endif
//  #else
//      #define EXPORT_API
//  #endif


#endif // LIBEXPORTS_H