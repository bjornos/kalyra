#pragma once

#include "firmwareRelease.hh"

/*#if defined(_WIN32) || defined(_WIN64)
constexpr auto SCRIPT_FETCH = "fetch_targets.bat";
constexpr auto SCRIPT_BUILD = "build_targets.bat";
constexpr auto SCRIPT_FETCH_CMD = "cmd.exe fetch_targets.bat";
constexpr auto SCRIPT_BUILD_CMD = "cmd.exe build_targets.bat";
#else*/


//#endif


class scriptGenerator {

public:
    static constexpr auto BUILDDIR = "workdir";
    static constexpr auto SCRIPT_FETCH = "workdir/fetch_targets.sh";
    static constexpr auto SCRIPT_BUILD = "workdir/build_targets.sh";
    static constexpr auto SCRIPT_RELEASE = "workdir/release_targets.bat";

    static constexpr auto SCRIPT_FETCH_CMD = "bash workdir/fetch_targets.sh";
    static constexpr auto SCRIPT_BUILD_CMD = "bash workdir/build_targets.sh";
    static constexpr auto SCRIPT_RELEASE_CMD = "cmd  /C workdir\\release_targets.bat";

    scriptGenerator();
    ~scriptGenerator();

    static void fetch(std::unique_ptr<firmwareRelease>& release);
    static void build(std::unique_ptr<firmwareRelease>& release);
    static void release(std::unique_ptr<firmwareRelease>& release);
};
