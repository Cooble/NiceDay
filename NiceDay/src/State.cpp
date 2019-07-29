#include "ndpch.h"
#include "Stats.h"
#include "world/WorldRenderManager.h"


bool Stats::light_enable=false;
bool Stats::gun_enable=false;
bool Stats::fly_enable=true;
bool Stats::move_through_blocks_enable =true;
float Stats::player_speed =0.5f;
float Stats::player_light_intensity = 1.0f;
float Stats::debug_x = 1.0f;
volatile int Stats::light_millis = 0;
BiomeDistances Stats::biome_distances = BiomeDistances();
