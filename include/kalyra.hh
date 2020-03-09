#pragma once

#define DBG(x) x

#define PLT_SHELL "bash"

#define BUILD_DIR "sources"
#define KALYRA_SCRIPT_DIR "scripts"
#define KALYRA_CONF_DIR "conf"
#define KALYRA_WORK_DIR "conf"


#define KALYRA_BANNER "Kalyra Build System"
#define KALYRA_MAJOR "0"
#define KALYRA_MINOR "1"
#define KALYRA_SUB	"2"


constexpr auto SCRIPT_FETCH_META = KALYRA_SCRIPT_DIR "/fetch_meta.sh";

constexpr auto SCRIPT_FETCH_RECIPE = KALYRA_SCRIPT_DIR "/fetch_recipe.sh";

constexpr auto CMD_SCRIPT_FETCH_META = KALYRA_SCRIPT_DIR "/fetch_meta.sh";

constexpr auto CMD_SCRIPT_FETCH_RECIPE = KALYRA_SCRIPT_DIR "/fetch_recipe.sh";




