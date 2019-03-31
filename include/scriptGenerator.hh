#pragma once

#include "firmwareRelease.hh"
#include "kalyra.hh"

#define FETCH "fetch_targets"
#define BUILD "build_targets"
#define RELEASE "release_targets"
#define GITTAG "gittag"

#define SCRIPT_FETCH BUILDDIR PLT_SLASH FETCH "." PLT_SUFFIX
#define SCRIPT_BUILD BUILDDIR PLT_SLASH BUILD "." PLT_SUFFIX
#define SCRIPT_RELEASE BUILDDIR PLT_SLASH RELEASE "." PLT_SUFFIX
#define SCRIPT_GITTAG BUILDDIR PLT_SLASH GITTAG "." PLT_SUFFIX

#define SCRIPT_CMD_FETCH PLT_SHELL " " SCRIPT_FETCH
#define SCRIPT_CMD_BUILD PLT_SHELL " " SCRIPT_BUILD
#define SCRIPT_CMD_RELEASE PLT_SHELL " " SCRIPT_RELEASE
#define SCRIPT_CMD_GITTAG PLT_SHELL " " SCRIPT_GITTAG


class scriptGenerator {
public:
    scriptGenerator();
    ~scriptGenerator();

    static void fetch(std::unique_ptr<firmwareRelease>& release, const std::string& singleTarget, bool update, const std::string& updateTarget);
    static void build(std::unique_ptr<firmwareRelease>& release, const std::string& singleTarget);
    static void release(std::unique_ptr<firmwareRelease>& release, const std::string& manifest);
    static void gitTag(std::unique_ptr<firmwareRelease>& release);
};
