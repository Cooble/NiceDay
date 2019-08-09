#pragma once
struct BiomeDistances;
class Stats
{
public:
	static bool light_enable;
	static bool gun_enable;
	static bool fly_enable;
	static volatile int light_millis;
	static bool move_through_blocks_enable;
	static float player_speed;
	static float player_light_intensity;
	static float debug_x;
	static BiomeDistances biome_distances;
	static float edge_scale;
};
