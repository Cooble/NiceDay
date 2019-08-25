# NiceDay

Let's make Terraria in C++!  
  
### Pictured changelog:

#### Basic tile render (03_05_2019)
![Alt text](screenshots/03_05_2019.png?raw=false "")

#### Corner tile render (06_05_2019)
![Alt text](screenshots/06_05_2019.png?raw=false "")

#### Lighting prototype with water-spill-algorithm (18_05_2019)
![Alt text](screenshots/18_05_2019.png?raw=false "")

#### Platforms, grass, sky (20_05_2019)
![Alt text](screenshots/20_05_2019.png?raw=false "")

#### Blocks, Walls, Background (26_05_2019)
![Alt text](screenshots/26_05_2019.png?raw=false "")

#### Colorful block corners, glass, procedural chunk generation (14_06_2019)
![Alt text](screenshots/14_06_2019.png?raw=false "")

#### Dynamic and Cached monochromatic lighting system (19_06_2019)  
- Cached lighting is only calculated onBlockChange  

![Alt text](screenshots/19_06_2019.png?raw=false "")

#### Multithreaded lights,  Inheritance Entity System (maybe will change to ECS in the future)(22_07_2019)  
- Added NBT to save and load entities. (No entity saving yet, though.)  
- Added basic physics system with polygon collision detection.  

In the pic: The first entity after Player was (who would have guessed...) TNT!  
![Alt Text](screenshots/22_07_2019_00.gif?raw=false "")  

In the pic: red Zombie attacks black Player.  (and then mysteriously disappears up in the sky)  
![Alt Text](screenshots/22_07_2019_01.gif?raw=false "")   

#### Smooth wallLight vs edgy blockLight, Day/Night cycle (12_08_2019)  
- Added dynamically created TextureAtlas.  
- Added painting and multiblock structure support.  

![Alt text](screenshots/12_08_2019.png?raw=false "")   
Oh, shoot!   
![Alt Text](screenshots/12_08_2019.gif?raw=false "") 
  
#### ParticleSystem, Trees, Flowers, PlayerSprite, Walking on steep floor, fullscreen (25_08_2019)  
- Ability to walk on blocks which are 1high without jumping.     
- Player has a walk animation.     
- Added entity health bar.  
- Crude ParticleSystem using BatchRenderer2D temporarily, will have its own renderer with shader in the future. Can do 3500 particles without a problem.  
- Added multiblock structure = tree and its generator. After placing a sapling block, sapling tile entity counts the worldtime and then calls TreeGen. Trees have random number of branches(dry or normal) and random corona.  
- Everybody loves flowers. and graaasss. (Sheep are in sight :D)  

![Alt text](screenshots/25_08_2019.png?raw=false "")  



  
 

  


