#pragma once

#include "firmwareRelease.hh"

#if defined(_WIN32) || defined(_WIN64)
#define PLT_SUFFIX "bat"
#define PLT_SHELL "cmd  /C"
#define PLT_SLASH "\\"
#else
#define PLT_SUFFIX "sh"
#define PLT_SHELL "bash "
#define PLT_SLASH "/"
#endif

#define FETCH "fetch_targets"
#define BUILD "build_targets"
#define RELEASE "release_targets"

#define BUILDDIR "workdir"
#define SCRIPT_FETCH BUILDDIR PLT_SLASH FETCH "." PLT_SUFFIX
#define SCRIPT_BUILD BUILDDIR PLT_SLASH BUILD "." PLT_SUFFIX
#define SCRIPT_RELEASE BUILDDIR PLT_SLASH RELEASE "." PLT_SUFFIX

#define SCRIPT_CMD_FETCH PLT_SHELL " " SCRIPT_FETCH
#define SCRIPT_CMD_BUILD PLT_SHELL " " SCRIPT_BUILD
#define SCRIPT_CMD_RELEASE PLT_SHELL " " SCRIPT_RELEASE


class scriptGenerator {
public:
    scriptGenerator();
    ~scriptGenerator();

    static void fetch(std::unique_ptr<firmwareRelease>& release);
    static void build(std::unique_ptr<firmwareRelease>& release);
    static void release(std::unique_ptr<firmwareRelease>& release);
};
