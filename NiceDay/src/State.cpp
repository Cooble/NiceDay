#include "ndpch.h"
#include "Stats.h"
#include "world/WorldRenderManager.h"
#include "world/World.h"


bool Stats::light_enable=true;
bool Stats::gun_enable=false;
bool Stats::fly_enable=true;
bool Stats::move_through_blocks_enable =true;
float Stats::player_speed =0.5f;
float Stats::player_light_intensity = 1.0f;
float Stats::debug_x = 1.0f;
volatile int Stats::light_millis = 0;
BiomeDistances Stats::biome_distances = BiomeDistances();
float Stats::edge_scale = 0.35;
int Stats::updates_per_frame = 0;
Sprite* Stats::bound_sprite = nullptr;
bool Stats::show_collisionBox = false;
World* Stats::world = nullptr;
int Stats::particle_count = 0;
