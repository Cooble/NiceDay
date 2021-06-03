#pragma once
//precompiled header file

// preprocessor defines
//
// defines not to use sol (lua binder library) (some things will not work)
// used because some versions of compiler seems to have problem compiling sol
// #define NOO_SOOL

//streams
#include <filesystem>
#include <iostream>
#include <sstream>
#include <fstream>

//data sets
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>
#include <set>

//threads
#include <mutex>

//glm
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/type_ptr.hpp>



//program specific
#include "core/Log.h"
#include "core/SUtil.h"
#include "core/Core.h"
#include "core/NDUtil.h"

//profiling
#include "core/Scoper.h"
#include "core/sids.h"


