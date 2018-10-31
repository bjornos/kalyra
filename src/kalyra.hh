#pragma once

#define KALYRA_MAJOR "1"
#define KALYRA_MINOR "0"
#define KALYRA_SUB  "1"
#define KALYRA_BANNER "Kalyra Build System"

#if defined(_WIN32) || defined(_WIN64)
#define PLT_SUFFIX "bat"
#define PLT_SHELL "cmd.exe  /C"
#define PLT_SLASH "\\"
#else
#define PLT_SUFFIX "sh"
#define PLT_SHELL "bash"
#define PLT_SLASH "/"
#endif
