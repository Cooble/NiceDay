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
- [Multithreaded lights,  Inheritance Entity System](#multithreaded-lights--inheritance-entity-system-maybe-will-change-to-ecs-in-the-future22_07_2019)
- [Smooth wallLight vs edgy blockLight, Day/Night cycle](#smooth-walllight-vs-edgy-blocklight-daynight-cycle-12_08_2019)
- [ParticleSystem, Trees, Flowers, PlayerSprite, Walking on steep floor, fullscreen](#particlesystem-trees-flowers-playersprite-walking-on-steep-floor-fullscreen-25_08_2019)  
- [Basic GUI module in engine](#basic-gui-module-in-engine-13_11_2019)  
- [Advanced GUI in engine](#advanced-gui-in-engine-23_11_2019)   
- [Lua, Items and XMas](#lua-items-and-xmas-24_12_2019)   
## Pictured changelog:

#### Basic tile render (03_05_2019)
![Alt text](03_05_2019.png?raw=false "")

#### Corner tile render (06_05_2019)
![Alt text](06_05_2019.png?raw=false "")

#### Lighting prototype with water-spill-algorithm (18_05_2019)
![Alt text](18_05_2019.png?raw=false "")

#### Platforms, grass, sky (20_05_2019)
![Alt text](20_05_2019.png?raw=false "")

#### Blocks, Walls, Background (26_05_2019)
![Alt text](26_05_2019.png?raw=false "")

#### Colorful block corners, glass, procedural chunk generation (14_06_2019)
![Alt text](14_06_2019.png?raw=false "")

#### Dynamic and Cached monochromatic lighting system (19_06_2019)  
- Cached lighting is only calculated onBlockChange  

![Alt text](19_06_2019.png?raw=false "")

#### Multithreaded lights,  Inheritance Entity System (maybe will change to ECS in the future)(22_07_2019)  
- Added NBT to save and load entities. (No entity saving yet, though.)  
- Added basic physics system with polygon collision detection.  

In the pic: The first entity after Player was (who would have guessed...) TNT!  
![Alt Text](22_07_2019_00.gif?raw=false "")  

In the pic: red Zombie attacks black Player.  (and then mysteriously disappears up in the sky)  
![Alt Text](22_07_2019_01.gif?raw=false "")   

#### Smooth wallLight vs edgy blockLight, Day/Night cycle (12_08_2019)  
- Added dynamically created TextureAtlas.  
- Added painting and multiblock structure support.  

![Alt text](12_08_2019.png?raw=false "")   
Oh, shoot!   
![Alt Text](12_08_2019.gif?raw=false "") 
  
#### ParticleSystem, Trees, Flowers, PlayerSprite, Walking on steep floor, fullscreen (25_08_2019)  
- Ability to walk on blocks which are 1high without jumping.     
- Player has a walk animation.     
- Added entity health bar.  
- Crude ParticleSystem using BatchRenderer2D temporarily, will have its own renderer with shader in the future. Can do 3500 particles without a problem.  
- Added multiblock structure = tree and its generator. After placing a sapling block, sapling tile entity counts the worldtime and then calls TreeGen. Trees have random number of branches(dry or normal) and random corona.  
- Everybody loves flowers. and graaasss. (Sheep are in sight :D)  

![Alt text](25_08_2019.png?raw=false "")    
  
#### Basic GUI module in engine (13_11_2019)  
- in order to start with inventory, one must create some sort of GUI before...   
- currently contains Windows (which can overlap), Label, Button, CheckBox, TextBox, HorizontalSlider and Image + layouts: (Column, Row, Grid)  
- There is one GUIContext which cares about parent-child structure and events of GUIElements and GUIRenderer, which renders those elements  
- positions are relative to the parental element  
- currently no resizability  
- to change a color, one has to change the GUIRenderer  

![Alt text](13_11_2019.png?raw=false "")  

#### Advanced GUI in engine (23_11_2019)  
- Added view, (which is rendered to another texture before being rendered on the screen)  
- Better text render, added kerning!  
- Vertical Slider  
- Horizontal and Vertical Split  
- Its really dynamic :D - resizable, ability to inherit dimensions from parent nodes  
- And last but not least..             HAPPY 100TH COMMIT!  Yay :D  

![Alt text](23_11_2019.gif?raw=false "")    

#### Lua, Items and XMas (24_12_2019)  
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
  
![Alt text](24_12_2019.png?raw=false "") 
