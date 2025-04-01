#ifndef UTILITY_API_H
#define UTILITY_API_H

#ifdef _WIN32
#ifdef UTILITY_EXPORTS
#define UTILITY_API __declspec(dllexport)
#else
#define UTILITY_API __declspec(dllimport)
#endif
#else
#define UTILITY_API
#endif

#endif // UTILITY_API_H