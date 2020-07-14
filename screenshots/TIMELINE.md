# NiceDay - CHANGELOG

Let's see how we're doing...
## TimeLine:
- [Basic Tile Render](#basic-tile-render-03_05_2019)
- [Corner tile render](#corner-tile-render-06_05_2019)
- [Lighting prototype with water-spill-algorithm](#lighting-prototype-with-water-spill-algorithm-18_05_2019)
- [Platforms, grass, sky](#platforms-grass-sky-20_05_2019)
- [Blocks, Walls, Background](#blocks-walls-background-26_05_2019)
- [Colorful block corners, glass, procedural chunk generation](#colorful-block-corners-glass-procedural-chunk-generation-14_06_2019)
- [Dynamic and Cached monochromatic lighting system](#dynamic-and-cached-monochromatic-lighting-system-19_06_2019)
- [Multithreaded lights,  Inheritance Entity System](#multithreaded-lights--inheritance-entity-system-22_07_2019)
- [Smooth wallLight vs edgy blockLight, Day/Night cycle](#smooth-walllight-vs-edgy-blocklight-daynight-cycle-12_08_2019)
- [ParticleSystem, Trees, Flowers, PlayerSprite, Walking on steep floor, fullscreen](#particlesystem-trees-flowers-playersprite-walking-on-steep-floor-fullscreen-25_08_2019)  
- [Basic GUI module in engine](#basic-gui-module-in-engine-13_11_2019)  
- [Advanced GUI in engine](#advanced-gui-in-engine-23_11_2019)   
- [Lua, Items and XMas](#lua-items-and-xmas-24_12_2019)  
- [The Great Sound Update](#the-great-sound-update-28_04_2020)     
- [JSONfication of Blocks, Docking](#jsonification-of-blocks-docking-14_07_2020)
## Pictured changelog:

### Basic tile render (03_05_2019)
![Alt text](03_05_2019.png?raw=false "")

### Corner tile render (06_05_2019)
![Alt text](06_05_2019.png?raw=false "")

### Lighting prototype with water-spill-algorithm (18_05_2019)
![Alt text](18_05_2019.png?raw=false "")

### Platforms, grass, sky (20_05_2019)
![Alt text](20_05_2019.png?raw=false "")

### Blocks, Walls, Background (26_05_2019)
![Alt text](26_05_2019.png?raw=false "")

### Colorful block corners, glass, procedural chunk generation (14_06_2019)
![Alt text](14_06_2019.png?raw=false "")

### Dynamic and Cached monochromatic lighting system (19_06_2019)  
- Cached lighting is only calculated onBlockChange  

![Alt text](19_06_2019.png?raw=false "")

### Multithreaded lights,  Inheritance Entity System (22_07_2019)  
- Added NBT to save and load entities. (No entity saving yet, though.)  
- Added basic physics system with polygon collision detection.  
- Maybe will change to ECS in the future

In the pic: The first entity after Player was (who would have guessed...) TNT!  
![Alt Text](22_07_2019_00.gif?raw=false "")  

In the pic: red Zombie attacks black Player.  (and then mysteriously disappears up in the sky)  
![Alt Text](22_07_2019_01.gif?raw=false "")   

### Smooth wallLight vs edgy blockLight, Day/Night cycle (12_08_2019)  
- Added dynamically created TextureAtlas.  
- Added painting and multiblock structure support.  

![Alt text](12_08_2019.png?raw=false "")   
Oh, shoot!   
![Alt Text](12_08_2019.gif?raw=false "") 
  
### ParticleSystem, Trees, Flowers, PlayerSprite, Walking on steep floor, fullscreen (25_08_2019)  
- Ability to walk on blocks which are 1high without jumping.     
- Player has a walk animation.     
- Added entity health bar.  
- Crude ParticleSystem using BatchRenderer2D temporarily, will have its own renderer with shader in the future. Can do 3500 particles without a problem.  
- Added multiblock structure = tree and its generator. After placing a sapling block, sapling tile entity counts the worldtime and then calls TreeGen. Trees have random number of branches(dry or normal) and random corona.  
- Everybody loves flowers. and graaasss. (Sheep are in sight :D)  

![Alt text](25_08_2019.png?raw=false "")    
  
### Basic GUI module in engine (13_11_2019)  
- in order to start with inventory, one must create some sort of GUI before...   
- currently contains Windows (which can overlap), Label, Button, CheckBox, TextBox, HorizontalSlider and Image + layouts: (Column, Row, Grid)  
- There is one GUIContext which cares about parent-child structure and events of GUIElements and GUIRenderer, which renders those elements  
- positions are relative to the parental element  
- currently no resizability  
- to change a color, one has to change the GUIRenderer  

![Alt text](13_11_2019.png?raw=false "")  

### Advanced GUI in engine (23_11_2019)  
- Added view, (which is rendered to another texture before being rendered on the screen)  
- Better text render, added kerning!  
- Vertical Slider  
- Horizontal and Vertical Split  
- Its really dynamic :D - resizable, ability to inherit dimensions from parent nodes  
- And last but not least..             HAPPY 100TH COMMIT!  Yay :D  

![Alt text](23_11_2019.gif?raw=false "")    

### Lua, Items and XMas (24_12_2019)  
- Even though adding scripting language to the engine was in DEEP FUTURE plans, once I realized how easy it would be to just rewrite one text line, hit enter and spawn particles with slightly tweaked velocities instead of recompiling the whole game again I finally decided that that time had come. Ladies and gentlemen, Brace yourself for...
  ##### The Story of adding Lua:
  Adding lua itself was the easiest thing, simply add statically native lua lib and that was it, then I used ConsoleWindow template found in imgui_demo.cpp and suddenly I could use Lua console within the game itself :D
  The real pain, unfortunately, had yet to come. To use it to interact with the engine and with a world eventually, I needed to register and hook some C++ methods to be accessible by lua. I immediately added ND_INFO() to print messages using native lua lib. So far so good. Then I realized that's too much of the work and that I would use some third-party libs that would bridge that gap for me.
  And with the biggest tool in the shed I went: **SWIG**.(Simplified Wrapper and Interface Generator). It generates C++ files called e.g. classname_wrapper.cxx or something. special method in those files needs to be called and then you have working lua-c++ interface Yay...
  For a while my desire was satisfied. Then I though about some more simple or elegant method (which was the begging of an end (my end)). I searched and found **luabind**, an awesome library that seems to provide simple interface to easily bind classes to lua as well (btw SWIG can do that too). I tried to compile it with the engine but compiler was so nice and politely informed me that I hadn't read the READMEs. Well this lib seems to need a Boost?. That was actually my first encounter with the vast library of libraries so I naively thought I could just include it in my project and guess what... It didn't work. Up to this day I still haven't find a way to successfully import it ("one day I swear I will beat you!"). With this option out of the way I tried the deboostified version of luabind for which the linker found the courage to tell me about his inability to found some symbol definition which I was deeply sure was happily contained in one cpp file. And so I dropped the idea of using luabind completely... Farewell. In the end after possibly 1 week of trials and errors I settled with an incredibly simple lib **LuaBridge** that doesn't need anything and uses templates. What a great adventure! LuaBridge helped me to bind glm::vectors to lua as well as method World::spawnParticle() and with that I could finally make a loop and spawn as many goddamned particles as I wanted. The End (at least for now)
  
- Finally the long awaited Items have arrived:  
- Items have unique string ids (uint64_t hashed versions are used to speed up the lookup time) using FNV-1a
- Added EntityItem which represents item dropped in the world  

- Apart from inventory system also HUD system was made:  
- HUD is a GUIWindow to which you can register GUIEntities  
- GUIEntities have nothing to do with world and can represent for example HealthBar, ManaBar, ActionInventorySlots, CraftingInventory, ChestSlots ...  
- It's important to note that its mandatory for gui system and world system to be "decoupled" (the world shouldn't care if HealthBar is showed) (on the other hand GUIEntity needs to communicate with World/entities)  
- This is due to the fact that in the future, there might be multiplayer where server will definitely not care about some puny pathetic GUI stuff   
- And about Christmas, SnowmanEntity can be spawned using pumpkin stacked onto a pile of snow. Slippery ice blocks were added as well. That's all for now. Happy XMAS.  
In the pic: ItemEntities circling around the player. You wouldn't have guessed how easy it is to get to the stable orbital position :D  
![Alt text](23_12_2019.gif?raw=false "ItemEntities circling around the player")  
  
![Alt text](24_12_2019.png?raw=false "Player Inventory, Snowman") 


### The Great Sound Update (28_04_2020)  
Well, it has been 5 months since last TIMELINE entry, but that doesn't mean that I haven't done anything, not even close!
As it's obvious from the title Engine now finally has full Sound support. There are three main parts to this madness:
1. **libogg** (for loading sound streams)
2. **libvorbis** (for decoding ogg sound streams)
3. and finally **portaudio** to play the data
4. and yeah of course the thing I wrote...

 Gotta admit that I was considering using some already available sound engines, mainly OpenAL but since it's not entirely free to use (API is, implementation is not) I decided to write my own. To put it simply there were few requirements:

 1. Play music = sound file isn't loaded into memory all at once (That would take looong time), instead it fills the buffer and after a part is consumed and played new data is inserted to the buffer from file, yeah, stream in the nutshell..
2. Play sound and cache it = It would be a pain to load sound every time one requests to play the same audio. If I had a machine gun (e.g. S.D.M.G) for example just imagine how many times the same .ogg file would need to be opened and read from at once! That's why it's important to store a decoded version of sound for later use if neccessary. When there is no more space we just drop the sound buffer which is not used.
3. Functions like ``setVolume, fadeIn, fadeOut, setSpeed/setPitch`` (same thing for now) and ``loop``.
4. Spatial sound - the farther away the radio is, the lower the volume. The more to the left the radio is, the lower the volume on the right channel 
  
  And I also added **AudioHandle** to interact with instead of command like methods of class Sounder (best naming ever). It's much more OOP.
  AudioHandle object can be also used in lua commands, so it's possible to write script that plays playlist!

```lua
function playFolder(fileP)
	local songs = ls(fileP) --list files (in startup.lua)
	local function basename(path)
  		return path:sub(path:find("/[^/]*$") + 1)
	end

  ND_INFO("Playing list of songs (" .. #songs .. " files)") --logging to console
  for index,value in ipairs(songs) do
        if not value.isDirectory and ends_with(value.path,".ogg") then	--ends_with() in startup.lua
            s = Music() --music handle
            s:open(value.path) --set filePath
            s:setVolume(0.5,0) --set volume=0.5 with fade_in_time = 0 seconds
            s:play(0) --start with fade_in_time = 0 seconds
			
            ND_INFO("playing " .. basename(value.path));
            while(s:isPlaying()) do
               waitFor(10) --from ND coroutine library, will wait for 10 game ticks
            end
        end
   end
end
playFolder("D:/SteamLibrary/steamapps/common/Undertale/") --quality content
```
But wait! There's more!
### Sun, Moon and Stars
It's unhealthy to stare directly into the sun. 

![Alt text](28_04_2020_dayNight.gif?raw=false "Day Night Cycle") 

And last but not least: The Whole project got ported from **premake** (which is awesome but nobody uses it) to **CMake** (with horrifying syntax but it works very well). To build the whole project only 2 commands need to be run:
```
- cmake .   #creates build files like .sln or make files
- cmake --build . --config Debug --target Sandbox   #builds the program 
```
Also the project is finally checked by Travis to ensure that each commit is working. (that was actually the reason why I transfered to CMake in the first place)

Listening to the **STATIC ON THE RADIO**
![Alt text](28_04_2020.gif?raw=false "Listening to the static on the radio") 

### JSONification of Blocks, Docking (14_07_2020)
The amount of data in blocks.cpp has become impossible to maintain.
I decided to outsource the data to json files located in
res/registry. There is no space in cpp code for that!
To add new block, one no longer needs to write an entry in header file as
well as cpp files.
To add a new block:
	1. add entry to blocks.ids (choose unique id for the block)
	2. add json map object to blocks.json
	3. fill the object with desired attributes (hardness,
	lightSrc...)
	4. CPP version can be still used nevertheless alongside with
	json: just specify blank constructor with string block_id, all
	of the attributes get copied from json file so one can focus solely
	on overriding functions.
  
- Note:
	CMake script automaticaly creates a special header file (from blocks.ids) that contains all
	block ids as constexpr int constants like BLOCK_DIRT so it's
	easy to use it in the code.
  
A block entry might look something like this:
```
 //note: added comment support even though JSON specs "forbid" it
 "stone": {
    "hardness": 2,
    "hasBigTexture": true,
    "connectGroups": "dirt",
    "corners": "dirt"
  },
 "radio": {
    "opacity": "air",
    "collision": false,
    "cannotFloat": true,
    "multiBlock": {
      "width": 2,
      "height": 2
    },
    "tileEntity": "TileEntityRadio",
    "flags": [ "burnable" ]
  },
  /* 
  Multiliners are also OK, Yay!
  */
```  
#### ImGui Docking
Finally I've switched to docking branch of imgui which makes docking as well as multiple viewports possible.
The imgui windows can be easily docked into the main window or can stay outside of it independently.
I had to change `App::getWindow()` to return not neccessarily the physical glfw window but also a "fake" one which is just proxy for another imgui window.
It depends on the app settings and can be switched off. (no docking no multi-viewports) 
Also since the default OPENGL FBO doesn't have to be the default render target anymore, -> 
##### (GL)FrameBuffer was completely redone:
- No longer is just a shell for textures (attachments)
- FrameBuffer has 3 modes: 
    - `WINDOW_TARGET` (render directly to physical application window)
    - `NORMAL_TARGET` with textures (has its own set of attachments which are created in the constructor)
    - `NORMAL_TARGET_EXTERNAL_TEXTURES` (one needs to manually attach textures = same as before this change)
 
When using FBOs the current approach is not to bind them but every
time something is renderered simply pass them to renderer, This helps with
troubleshooting to find which damned FBO I am rendering to right now. ('cause God bless OPENGL state machine)
##### Also...
- Added very crude scene support (which will be probably redone using ECS in the future) 
- Added assimp for 3D model loading
- Simple 3D lighting
- 3D Cameras (Player and Editor)
- Added Mandelbrot shader, just for fun (also Mandelbulb (though only slices can be viewed))


![Alt text](09_07_2020_scene_layout.png?raw=false "Scene Layout") 

