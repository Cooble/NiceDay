# NiceDay
[![Build Status](https://travis-ci.com/Cooble/NiceDay.svg?branch=master)](https://travis-ci.com/Cooble/NiceDay)

Let's make Terraria in C++!  
I decided to make a "simple" clone of Terraria in C++ using my own engine which is being created on the go. 
<br> 
<br> 
**!!To see how project progresses see** [**changelog with pictures**](screenshots/TIMELINE.md)**!!** ("*it's interesting*")
<br>  
<br>
![Alt text](screenshots/back_logo.png?raw=false "logo")

I decided to create this game/engine in order to improve my programming skills in C++ and see, 
how "easy" it is to recreate such a popular 2D pixelart-ish game in C++ with only a "few" 3rd party libraries. 
Well, if there is a hell in the world... I think I've found it.<br>
Also this project focuses on Terraria clone as well as on "universal" 3D engine, 
so updates in [TIMELINE.md](screenshots/TIMELINE.md) are from both.
And yes, I like to use '"' a lot.

## Installation
For now the whole clone-and-run proccess should as simple as:
1. `git clone --recurse-submodules (--depth=1) https://github.com/Cooble/NiceDay.git`
2. In parallel can be run:
   - `External-WIN32-Build.bat` (create build directory)
   - `DownloadAdditionalResources.bat` (download resources that are not part of git VCS)
   - `InstallMono.bat` (if Mono is not yet installed on the machine)
3. `cd build`
3. `start NiceDaySolution.sln` or `cmake --build . --config (Debug | Release) --target (Sandbox | SandboxTest | TestNiceDay)`


### External dependencies/libs
- The engine is inspired by: https://github.com/TheCherno/Hazel   
- GLM https://github.com/g-truc/glm
- ASSIMP https://github.com/assimp/assimp
- SPDLOG https://github.com/gabime/spdlog

##### ==Graphics==
- GLFW https://github.com/glfw/glfw
- GLAD https://glad.dav1d.de/
- IMGUI https://github.com/ocornut/imgui
- IMGUI_FILE_DIALOG https://github.com/aiekick/ImGuiFileDialog
- STB https://github.com/nothings/stb

##### ==Sound==
- OGG https://github.com/xiph/ogg
- VORBIS https://github.com/xiph/vorbis
- PORT_AUDIO https://git.assembla.com/portaudio.git

##### ==Other langs==
- MONO https://github.com/mono/mono
- LUA https://www.lua.org/download.html
- SOL2 https://github.com/ThePhD/sol2
- NLOHMANN_JSON https://github.com/nlohmann/json
- RAPID_XML http://rapidxml.sourceforge.net/
