#pragma once

#ifdef ENG_LOG_ENABLED
    #include <cstdio>
    #define ENG_LOG(message) std::printf("%s\n", message)
    #define ENG_LOG_F(format_string, ...) std::printf(format_string, __VA_ARGS__); std::printf("\n")
#else
    #define ENG_LOG
    #define ENG_LOG_F
#endif