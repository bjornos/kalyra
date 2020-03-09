
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <nlohmann/json.hpp>
#include "termcolor/termcolor.hpp"
#include "input_parser.hh"
#include "release.hh"
#include "script_generator.hh"
#include "kalyra.hh"

using namespace std;
using json = nlohmann::json;




