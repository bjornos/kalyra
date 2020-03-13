#pragma once

#include <stdexcept>

#define DBG(x)

//#define PLT_SHELL "bash"

#define BUILD_DIR "sources"
#define KALYRA_SCRIPT_DIR "scripts"
#define KALYRA_CONF_DIR "conf"
#define KALYRA_WORK_DIR "conf"


#define KALYRA_BANNER "Kalyra Build System"
#define KALYRA_MAJOR "0"
#define KALYRA_MINOR "1"
#define KALYRA_SUB	"2"


constexpr auto LOG_BUILD_REVISIONS = "revisions.txt";



#if defined(_WIN32) || defined(_WIN64)

#define PLT_SLASH "\\"
//constexpr auto PLT_SLASH = "\\";


constexpr auto SCRIPT_FETCH_META = KALYRA_SCRIPT_DIR "\\fetch_meta.bat";

constexpr auto SCRIPT_FETCH_RECIPE = KALYRA_SCRIPT_DIR "\\fetch_recipe.bat";

constexpr auto CMD_SCRIPT_FETCH_META = KALYRA_SCRIPT_DIR "\\fetch_meta.bat";

constexpr auto CMD_SCRIPT_FETCH_RECIPE = KALYRA_SCRIPT_DIR "\\fetch_recipe.bat";

#else

#define PLT_SLASH  "/"

constexpr auto SCRIPT_FETCH_META = KALYRA_SCRIPT_DIR "/fetch_meta.sh";

constexpr auto SCRIPT_FETCH_RECIPE = KALYRA_SCRIPT_DIR "/fetch_recipe.sh";

constexpr auto CMD_SCRIPT_FETCH_META = KALYRA_SCRIPT_DIR "/fetch_meta.sh";

constexpr auto CMD_SCRIPT_FETCH_RECIPE = KALYRA_SCRIPT_DIR "/fetch_recipe.sh";
#endif



