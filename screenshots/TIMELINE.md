# NiceDay

Let's make Terraria in C++!  
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


