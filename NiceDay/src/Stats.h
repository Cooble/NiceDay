#pragma once
struct BiomeDistances;
class Stats
{
public:
	static bool light_enable;
	static volatile int light_millis;
	static bool move_through_blocks_enable;
	static float player_speed;
	static float player_light_intensity;
	static BiomeDistances biome_distances;
};
