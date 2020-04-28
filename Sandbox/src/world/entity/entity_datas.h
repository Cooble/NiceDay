#pragma once
#include "EntityManager.h"

constexpr EntityType ENTITY_TYPE_PLAYER = 0;
constexpr EntityType ENTITY_TYPE_TNT = 1;
constexpr EntityType ENTITY_TYPE_ZOMBIE = 2;
constexpr EntityType ENTITY_TYPE_ROUND_BULLET = 3;
constexpr EntityType ENTITY_TYPE_TILE_ENTITY = 4;
constexpr EntityType ENTITY_TYPE_TILE_SAPLING = 5;
constexpr EntityType ENTITY_TYPE_TILE_TORCH = 6;
constexpr EntityType ENTITY_TYPE_SNOWMAN = 7;
constexpr EntityType ENTITY_TYPE_TILE_CHEST = 8;
constexpr EntityType ENTITY_TYPE_ITEM = 9;
constexpr EntityType ENTITY_TYPE_BOMB = 10;
constexpr EntityType ENTITY_TYPE_TILE_RADIO = 11;




constexpr EntityType ENTITY_TYPE_NONE = std::numeric_limits<uint32_t>::max();