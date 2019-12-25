%module customodul
%include "std_string.i"
 %{
 /* Put header files here or function declarations like below */
	#include "LuaExampel.h"
 %}
/*%apply const std::string& {std::string* foo};*/
%include "LuaExampel.h"
