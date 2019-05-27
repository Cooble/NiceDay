#pragma once
#include "ndpch.h"
#define BLOCK_GROUP_AIR_BIT				0
#define BLOCK_GROUP_DIRT_BIT			1
#define BLOCK_GROUP_PLATFORM_BIT		2
#define BLOCK_GROUP_ORE_BIT				3

const int BLOCK_AIR = 0;
const int BLOCK_STONE = 1;
const int BLOCK_DIRT = 2;
const int BLOCK_GOLD = 3;
const int BLOCK_ADAMANTITE = 4;
const int BLOCK_PLATFORM = 5;
const int BLOCK_GRASS = 6;
const int BLOCK_GLASS = 7;

const int WALL_AIR = 0;
const int WALL_DIRT = 1;
const int WALL_STONE = 2;


const half_int BLOCK_CORNERS_DIRT[16] = {
	half_int(0,0),//BLOCK_STATE_FULL				
	half_int(0,6),//BLOCK_STATE_LINE_UP				
	half_int(0,5),//BLOCK_STATE_LINE_DOWN			
	half_int(0,4),//BLOCK_STATE_LINE_LEFT			
	half_int(0,3),//BLOCK_STATE_LINE_RIGHT			
	half_int(4,5),//BLOCK_STATE_CORNER_UP_LEFT		
	half_int(5,5),//BLOCK_STATE_CORNER_UP_RIGHT		
	half_int(4,4),//BLOCK_STATE_CORNER_DOWN_LEFT	
	half_int(5,4),//BLOCK_STATE_CORNER_DOWN_RIGHT	
	half_int(0,7),//BLOCK_STATE_BIT					
	half_int(5,6),//BLOCK_STATE_LINE_HORIZONTAL		
	half_int(4,6),//BLOCK_STATE_LINE_VERTICAL		
	half_int(4,7),//BLOCK_STATE_LINE_END_UP			
	half_int(5,7),//BLOCK_STATE_LINE_END_LEFT		
	half_int(6,7),//BLOCK_STATE_LINE_END_DOWN		
	half_int(7,7),//BLOCK_STATE_LINE_END_RIGHT		
};
const half_int BLOCK_CORNERS_GLASS[16] = {
	half_int(0,0),//BLOCK_STATE_FULL				
	half_int(0,6),//BLOCK_STATE_LINE_UP				
	half_int(0,5),//BLOCK_STATE_LINE_DOWN			
	half_int(0,4),//BLOCK_STATE_LINE_LEFT			
	half_int(0,3),//BLOCK_STATE_LINE_RIGHT			
	half_int(4,5),//BLOCK_STATE_CORNER_UP_LEFT		
	half_int(5,5),//BLOCK_STATE_CORNER_UP_RIGHT		
	half_int(4,4),//BLOCK_STATE_CORNER_DOWN_LEFT	
	half_int(5,4),//BLOCK_STATE_CORNER_DOWN_RIGHT	
	half_int(0,7),//BLOCK_STATE_BIT					
	half_int(5,6),//BLOCK_STATE_LINE_HORIZONTAL		
	half_int(4,6),//BLOCK_STATE_LINE_VERTICAL		
	half_int(4,7),//BLOCK_STATE_LINE_END_UP			
	half_int(5,7),//BLOCK_STATE_LINE_END_LEFT		
	half_int(6,7),//BLOCK_STATE_LINE_END_DOWN		
	half_int(7,7),//BLOCK_STATE_LINE_END_RIGHT		
};
const half_int WALL_CORNERS_DIRT[] = {
	half_int(0,0),//BLOCK_STATE_FULL				
	half_int(3,2),//BLOCK_STATE_LINE_UP				
	half_int(3,0),//BLOCK_STATE_LINE_DOWN			
	half_int(2,1),//BLOCK_STATE_LINE_LEFT			
	half_int(4,1),//BLOCK_STATE_LINE_RIGHT			
	half_int(2,2),//BLOCK_STATE_CORNER_UP_LEFT		
	half_int(4,2),//BLOCK_STATE_CORNER_UP_RIGHT		
	half_int(2,0),//BLOCK_STATE_CORNER_DOWN_LEFT	
	half_int(4,0),//BLOCK_STATE_CORNER_DOWN_RIGHT			
};
