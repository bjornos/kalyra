#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "cJSON/cJSON.h"
#include "termcolor/termcolor.hpp"
#include "inputParser.hh"

using namespace std;


int main(int argc, char *argv[])
{
    cout << "recipe maker alive!" << endl;

	return EXIT_SUCCESS;
}
