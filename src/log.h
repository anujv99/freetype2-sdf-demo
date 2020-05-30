
// Simple logging system (basically just macros)

#ifndef _LOG_H_
#define _LOG_H_

#include <cstdio>

#define LOG_INFO(...)	fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n")
#define LOG_ERROR(...)	fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n")

#endif //_LOG_H_