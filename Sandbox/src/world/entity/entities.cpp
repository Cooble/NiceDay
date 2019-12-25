#include "ndpch.h"
#include "entities.h"
#include "graphics/BatchRenderer2D.h"
#include "entity_datas.h"
#include "graphics/API/Texture.h"
#include "world/block/BlockRegistry.h"
#include "world/World.h"
#include "world/block/block_datas.h"
#include "world/block/basic_blocks.h"

#include "core/Stats.h"
#include "core/App.h"
#include "graphics/Sprite.h"
#include "world/gen/TreeGen.h"
#include "world/particle/particles.h"
#include "gui/HUD.h"
#include "gui/GUIEntityChest.h"
#include "graphics/TextureAtlas.h"

using namespace glm;

//PhysEntity======================================================


//return if we have intersection
inline static bool findBlockIntersection(World& w, const Phys::Vect& entityPos, const Phys::Polygon& entityBound)
{
	auto& entityRectangle = entityBound.getBounds();
	for (int x = entityRectangle.x0 - 1; x < ceil(entityRectangle.x1) + 1; ++x)
	{
		for (int y = entityRectangle.y0 - 1; y < ceil(entityRectangle.y1) + 1; ++y)
		{
			Phys::Vect blockPos(x, y);
			blockPos += entityPos;

			auto stru = w.getBlock((int)blockPos.x, (int)blockPos.y);
			if (stru == nullptr || stru->block_id == BLOCK_AIR)
				continue;
			auto& block = BlockRegistry::get().getBlock(stru->block_id);
			if (!block.hasCollisionBox())
				continue;
			auto blockBounds = block.getCollisionBox((int)blockPos.x, (int)blockPos.y, *stru).copy();

			blockBounds.plus({x - (entityPos.x - (int)entityPos.x), y - (entityPos.y - (int)entityPos.y)});
			if (Phys::isIntersects(blockBounds, entityBound))
				return true;
		}
	}
	return false;
}

inline static float getFloorHeight(World& w, const Phys::Vect& entityPos, const Phys::Rectangle& entityRectangle)
{
	float lineY = std::numeric_limits<float>::lowest();
	for (int i = 0; i < 2; ++i)
	{
		for (int yy = 0; yy < 2; ++yy)
		{
			float x = entityRectangle.x0 + entityPos.x + i * entityRectangle.width();
			float y = entityRectangle.y0 + entityPos.y + yy;
			auto& pointDown0 = Phys::Vect(x, y);
			auto& pointDown1 = Phys::Vect(x, y - 10000);

			auto stru = w.getBlock((int)pointDown0.x, (int)pointDown0.y);
			if (stru == nullptr || stru->block_id == BLOCK_AIR)
				continue;
			auto& block = BlockRegistry::get().getBlock(stru->block_id);
			if (!block.hasCollisionBox())
				continue;

			auto blockBounds = block.getCollisionBox((int)pointDown0.x, (int)pointDown0.y, *stru).copy();
			auto blockPos = Phys::Vect((int)pointDown0.x, (int)pointDown0.y);
			for (int j = 0; j < blockBounds.size(); ++j)
			{
				auto& v0 = blockBounds[j] + blockPos;
				auto& v1 = blockBounds[(j + 1) % blockBounds.size()] + blockPos;

				if (i == 0) //left
				{
					if (v0.x > pointDown0.x)
						lineY = max(lineY, v0.y);
				}
				else if (v0.x < pointDown0.x) //right
					lineY = max(lineY, v0.y);

				auto v = Phys::intersectLines(v0, v1, pointDown0, pointDown1);
				if (v.isValid() && v.y < entityRectangle.y1 + entityPos.y)
					lineY = max(lineY, v.y);
			}
		}
	}
	return lineY;
}

constexpr float maxDistancePerStep = 0.4f;

static float sgn(float f)
{
	if (f == 0)
		return 0;
	return abs(f) / f;
}

void PhysEntity::computePhysics(World& w)
{
	auto& collidingEntities = w.getLoadedEntities();
	//todo make entity flags to dtermine whether this force oughta be applied
	float xRepelForce = 0;


	for (auto id : collidingEntities)
	{
		WorldEntity* e = w.getEntityManager().entity(id);
		auto collidier = dynamic_cast<PhysEntity*>(e);
		if (collidier && e->getID() != getID()&&e->hasFlag(EFLAG_COLLIDER))
			if (Phys::isIntersects(collidier->getCollisionBox(), collidier->getPosition(), m_bound, m_pos))
			{
				float maxDistance = collidier->getCollisionBox().getBounds().width() + m_bound.getBounds().width();
				maxDistance /= 2;
				float deltaX = m_pos.x - collidier->getPosition().x;
				xRepelForce += sgn(deltaX) * max(1 - abs(deltaX) / maxDistance, 0.5f);
			}
	}
	float lastXAcc = m_acceleration.x;
	if (xRepelForce != 0)
	{
		m_acceleration.x += clamp(xRepelForce, -1.f, 1.f) * 0.15f;
	}

	computeVelocity(w);

	m_acceleration.x = lastXAcc;

	computeWindResistance(w);
	float lengthCab = Phys::asVect(m_velocity).lengthCab();

	if (lengthCab > maxDistancePerStep)
	{
		int dividor = ceil(lengthCab / maxDistancePerStep);
		for (int i = 0; i < dividor; ++i)
		{
			if (moveOrCollide(w, 1.0f / dividor))
				break;
		}
	}
	else moveOrCollide(w, 1);
}

bool PhysEntity::moveOrCollide(World& w, float dt)
{
	m_blockage = NONE;
	m_is_on_floor = false;

	bool isSlippery = (w.getBlock(m_pos.x, m_pos.y - 1) != nullptr && w.getBlock(m_pos.x, m_pos.y - 1)->block_id ==
			BLOCK_ICE)
		|| (w.getBlock(m_pos.x, m_pos.y) != nullptr && w.getBlock(m_pos.x, m_pos.y)->block_id == BLOCK_ICE);
	float floorResistance = isSlippery ? 0.005 : 0.2; //negative numbers mean conveyor belt Yeah!


	//collision detection============================
	auto possibleEntityPos = m_pos + m_velocity * dt;

	//check if we can procceed in x, y direction
	if (findBlockIntersection(w, possibleEntityPos, m_bound))
	{
		possibleEntityPos = m_pos;
		possibleEntityPos.x += m_velocity.x * dt;
		//check if we can procceed at least in x direction
		if (findBlockIntersection(w, possibleEntityPos, m_bound))
		{
			//walk on floor check
			if (m_can_walk && m_bound.isRectangle)
			{
				float y0 = m_bound.getBounds().y0 + m_pos.y;
				float floorHeight = getFloorHeight(w, possibleEntityPos, m_bound.getBounds());
				float difference = floorHeight - y0;

				constexpr float maxHeightToWalk = 0.99f;
				if (floorHeight < (m_bound.getBounds().y1 + m_pos.y) &&
					difference > -0.06f &&
					difference < maxHeightToWalk)
				{
					possibleEntityPos.y = floorHeight + 0.05f + m_bound.getBounds().y0;
					if (!findBlockIntersection(w, possibleEntityPos, m_bound))
					{
						m_is_on_floor = true;
						m_pos = possibleEntityPos;
						m_velocity.y = 0;

						if (m_velocity.x > 0) //apply resistance
						{
							m_velocity.x -= floorResistance / 2;
							if (m_velocity.x < 0)
								m_velocity.x = 0;
						}
						else if (m_velocity.x < 0)
						{
							m_velocity.x += floorResistance / 2;
							if (m_velocity.x > 0)
								m_velocity.x = 0;
						}

						return false;
					}
				}
			}

			possibleEntityPos = m_pos;
			possibleEntityPos.y += m_velocity.y * dt;
			//check if we can procceed at least in y direction
			if (findBlockIntersection(w, possibleEntityPos, m_bound))
			{
				//we have nowhere to go
				m_blockage = m_velocity.x > 0 ? RIGHT : LEFT;
				if (m_velocity.y <= 0)
				{
					m_is_on_floor = true;
					m_pos.y = (int)m_pos.y + 0.01f;
				}
				possibleEntityPos = m_pos;
				m_velocity = {0, 0};
			}
			else
			{
				//we can proceed in y direction
				m_velocity.x = 0;
			}
		}
		else //we can procceed in x direction
		{
			if (m_velocity.y <= 0)
			{
				if (m_bound.isRectangle)
				{
					float floorHeight = getFloorHeight(w, possibleEntityPos, m_bound.getBounds());
					if (Phys::isValidFloat(floorHeight) && w.isBlockValid(possibleEntityPos.x, floorHeight) && abs(
						floorHeight - m_pos.y) < 0.5)
						possibleEntityPos.y = floorHeight + 0.01f;
				}
				m_is_on_floor = true;
			}
			m_velocity.y = 0;


			if (m_velocity.x > 0)
			{
				m_velocity.x -= floorResistance;
				if (m_velocity.x < 0)
					m_velocity.x = 0;
			}
			else if (m_velocity.x < 0)
			{
				m_velocity.x += floorResistance;
				if (m_velocity.x > 0)
					m_velocity.x = 0;
			}
		}
	}
	m_pos = possibleEntityPos;
	return false;
}

bool PhysEntity::moveOrCollideOnlyBlocksNoBounds(World& w)
{
	auto possiblePos = m_pos + m_velocity;
	auto b = w.getBlock(possiblePos.x, possiblePos.y);
	if(b)
	{
		auto& block = BlockRegistry::get().getBlock(b->block_id);
		if (block.hasCollisionBox()) {
			auto& blockBounds = block.getCollisionBox((int)possiblePos.x, (int)possiblePos.y, *b);
			if (Phys::contains(blockBounds, { (possiblePos.x - (int)possiblePos.x),(possiblePos.y - (int)possiblePos.y) }))
				return false;
		}
	}
	m_pos = possiblePos;
	return true;
}

void PhysEntity::computeVelocity(World& w)
{
	//motion=========================================
	m_velocity += m_acceleration;
	m_velocity = glm::clamp(m_velocity, -m_max_velocity, m_max_velocity);
}

void PhysEntity::computeWindResistance(World& w,float windResistance)
{
	//wind resistance================================
	//constexpr float windResistance = 0.01;
	constexpr float windVelocityDepenceFactor = 1000000; //we dont need higher air resistance the bigger the velocity

	if (m_velocity.x > 0)
	{
		m_velocity.x -= windResistance + m_velocity.x / windVelocityDepenceFactor;
		if (m_velocity.x < 0)
			m_velocity.x = 0;
	}
	else if (m_velocity.x < 0)
	{
		m_velocity.x += windResistance - m_velocity.x / windVelocityDepenceFactor;
		if (m_velocity.x > 0)
			m_velocity.x = 0;
	}
}

void PhysEntity::save(NBT& src)
{
	WorldEntity::save(src);
	src.set("velocity", m_velocity);
	src.set("acceleration", m_acceleration);
}

void PhysEntity::load(NBT& src)
{
	WorldEntity::load(src);
	m_velocity = src.get("velocity", glm::vec2(0, 0));
	m_acceleration = src.get("acceleration", glm::vec2(0, -9.8f / 60));
}

//Bullet======================================================

bool Bullet::checkCollisions(World& w, float dt)
{
	m_pos += m_velocity * dt;
	m_angle = Phys::asVect(m_velocity).angleDegrees();
	auto blokk = w.getBlock(m_pos.x, m_pos.y);
	if (blokk)
	{
		auto& blok = *blokk;
		if (!blok.isAir())
		{
			auto& blokTemplate = BlockRegistry::get().getBlock(blok.block_id);
			if (blokTemplate.hasCollisionBox() &&
				Phys::contains(blokTemplate.getCollisionBox(m_pos.x, m_pos.y, blok),
				               Phys::Vect(m_pos.x - (int)m_pos.x, m_pos.y - (int)m_pos.y)))
			{
				if (onBlockHit(w, m_pos.x, m_pos.y))
					return true;
			}
		}
	}
	else
	{
		//todo remove kill on unloaded chunks
		markDead(); //live in chunk that is not loaded - kill it
		return true;
	}
	auto& entities = w.getLoadedEntities();
	int nulls = 0;
	for (auto entityID : entities)
	{
		auto e = w.getLoadedEntity(entityID);
		if (e == nullptr)
			nulls++;
	}

	//ASSERT(nulls==0, "Shi");

	for (auto entityID : entities)
	{
		auto e = w.getLoadedEntity(entityID);
		if (e)
			if (glm::distance2(e->getPosition(), m_pos) < MAX_BULLET_ENTITY_DISTANCE_SQ)
				if (auto d = dynamic_cast<PhysEntity*>(e))
				{
					if (dynamic_cast<Bullet*>(d))
						continue; //no interaction with other bullets
				
					if (e->getID() != m_owner_id && d->getCollisionBox().size() != 0 &&
						Phys::contains(d->getCollisionBox(), Phys::asVect(m_pos - d->getPosition())))
					{
						if (onEntityHit(w, e))
							return true;
					}
				}
	}
	return false;
}

void Bullet::update(World& w)
{
	if (m_ticks_to_ignore_owner)
	{
		m_ticks_to_ignore_owner--;
	}
	else m_owner_id = ENTITY_ID_INVALID;
	
	if (m_live_ticks++ == m_max_live_ticks)
	{
		markDead();
		return;
	}
	PhysEntity::computeVelocity(w);
	auto lengthCab = Phys::asVect(m_velocity).lengthCab();
	if (lengthCab > maxDistancePerStep)
	{
		int dividor = ceil(lengthCab / maxDistancePerStep);
		for (int i = 0; i < dividor; ++i)
		{
			if (checkCollisions(w, 1.0f / dividor))
				break;
		}
	}
	else checkCollisions(w, 1);
}


bool Bullet::onBlockHit(World& w, int blockX, int blockY)
{
	markDead();
	return true;
}

bool Bullet::onEntityHit(World& w, WorldEntity* entity)
{
	markDead();
	return true;
}

void Bullet::render(BatchRenderer2D& renderer)
{
	auto m = glm::translate(glm::mat4(1.f), glm::vec3(m_pos.x, m_pos.y, 0));
	m = glm::rotate(m, Phys::toRad(m_angle), glm::vec3(0, 0, 1));
	renderer.push(m);
	renderer.submit(m_sprite);
	renderer.pop();
}

constexpr float entityItemGravityLower = -0.6f/60;

static Texture* s_atlas_texture;
constexpr float atlasBit = 1.f / 8;
static void loadItemAtlas()
{
	static bool loaded = false;
	if (!loaded) {
		loaded = true;
		s_atlas_texture = Texture::create(TextureInfo("res/images/itemAtlas/atlas.png").filterMode(TextureFilterMode::LINEAR));
	}
}

constexpr float entityItemMaxVelocityNormal = 10.f / 60;
constexpr float entityItemMaxVelocityFaster = 25.f / 60;

constexpr float entityItemGravityNormal = 15.f / 60;

EntityItem::EntityItem()
{
	m_max_live_ticks = 60 * 60;
	m_max_velocity = glm::vec2(entityItemMaxVelocityNormal);
	m_acceleration = { 0, entityItemGravityLower };

	m_bound = Phys::toPolygon({ -0.25, -0.5, 0.5, 0.5 });
	loadItemAtlas();
}

void EntityItem::setItemStack(ItemStack* stack)
{
	
	m_item_stack = stack;
	
	if (stack == nullptr)
		return;
	auto& item = stack->getItem();

	half_int txtOffset = item.getTextureOffset(*stack);

	m_sprite = UVQuad::build({ txtOffset.x * atlasBit,txtOffset.y * atlasBit }, { atlasBit ,atlasBit });
}




void EntityItem::update(World& w)
{
	/*long long now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	ND_INFO("between update {}", now - lastTime);
	lastTime = now;*/
	
	constexpr float maxDistance = 5;
	constexpr float minDistance = 1;
	constexpr  float accelerationTowardsOwner = 0.1;
	//if there is no one we can go to
	if (m_target == ENTITY_ID_INVALID)
	{
		m_acceleration = { 0,entityItemGravityLower };
		auto& entities = w.getEntitiesInRadius(m_pos, maxDistance);
		for (auto entity : entities)
			if (entity->getID()!=m_ignore_target && entity->isItemConsumer() && entity->wantsItem(m_item_stack))
			{
				m_target = entity->getID();
				goto FROWARD;
			}
	}
	else 
	{
		FROWARD:
		auto e = w.getEntityManager().entity(m_target);
		//add half block up to compensate for player's loc on the ground
		auto pos = e->getPosition()+glm::vec2(0,0.5f);
		float dist2 = glm::distance2(m_pos, pos);
		if(e==nullptr||dist2> maxDistance*maxDistance)
		{
			m_target = ENTITY_ID_INVALID;
		}else
		{
			m_speed_mode_ticks_remaining = 10;//still in the speed mode
			//we are sufficiently close to future owner
			if(dist2< minDistance* minDistance)
			{
				m_item_stack = e->consumeItem(m_item_stack);
				//we can die
				if(m_item_stack==nullptr)
				{
					m_target = ENTITY_ID_INVALID;
					markDead();
					return;
				}
				//owner didnt take everything :(
				else
				{
					m_target = ENTITY_ID_INVALID;
				}
			}
			auto force = pos-m_pos;
			m_acceleration = glm::normalize(force)* accelerationTowardsOwner;
		}
	}
	
	if(m_speed_mode_ticks_remaining)
	{
		m_speed_mode_ticks_remaining--;
		m_max_velocity = glm::vec2(entityItemMaxVelocityFaster);
	}
	else
	{
		m_ignore_target = ENTITY_ID_INVALID;
		m_max_velocity = glm::vec2(entityItemMaxVelocityNormal);
		m_acceleration.y = entityItemGravityLower;
	}
	PhysEntity::computeVelocity(w);
	PhysEntity::computeWindResistance(w, m_speed_mode_ticks_remaining?0.003f:0.01);
	if(!PhysEntity::moveOrCollideOnlyBlocksNoBounds(w)&&m_speed_mode_ticks_remaining)
	{
		m_max_velocity = glm::vec2(entityItemMaxVelocityNormal);
		PhysEntity::moveOrCollideOnlyBlocksNoBounds(w);
		m_max_velocity = glm::vec2(entityItemMaxVelocityFaster);
	}
}

void EntityItem::render(BatchRenderer2D& renderer)
{
	renderer.push(glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x, m_pos.y, 0)));
	renderer.submitTextureQuad({ -1.f,-0.5f,0 }, { 2.f,2.f }, m_sprite, s_atlas_texture);
	renderer.pop();
}

EntityType EntityItem::getEntityType() const
{
	return ENTITY_TYPE_ITEM;
}

void EntityItem::save(NBT& src)
{
	PhysEntity::save(src);
	NBT t;
	m_item_stack->serialize(t);
	src.set("item", t);
}

void EntityItem::load(NBT& src)
{
	PhysEntity::load(src);
	setItemStack(ItemStack::deserialize(src.get<NBT>("item")));
}

void EntityItem::onSpawned(World& w)
{
	m_speed_mode_ticks_remaining = 30;
}

Bullet::Bullet()
	: m_damage(0),
	  m_live_ticks(0),
	  m_max_live_ticks(60 * 30),
	  m_angle(0)
{
	m_flags |= EFLAG_TEMPORARY;
}

void Bullet::fire(float angle, float velocity)
{
	m_velocity.x = cos(angle) * velocity;
	m_velocity.y = sin(angle) * velocity;
}

void Bullet::fire(const glm::vec2& target, float velocity)
{
	m_velocity = glm::normalize(target - m_pos) * velocity;
}

//RoundBullet======================================================
EntityRoundBullet::EntityRoundBullet()
{
	m_max_live_ticks = 60 * 60;
	m_max_velocity = {100.f / 60, 100.f / 60};
	m_acceleration = {0, -1.f / 60.f};
	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/player.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 4, 4);
	m_sprite = Sprite(&res);
	m_sprite.setSpriteIndex(3, 0);
	float size = 1.5f;
	m_sprite.setPosition(glm::vec3(-size / 2, -size / 2, 0));
	m_sprite.setSize(glm::vec2(size, size));
	m_damage = 1.f;
}

EntityType EntityRoundBullet::getEntityType() const
{
	return ENTITY_TYPE_ROUND_BULLET;
}

bool EntityRoundBullet::onEntityHit(World& w, WorldEntity* entity)
{
	auto en = dynamic_cast<PhysEntity*>(entity);
	if (en)
	{
		auto punchBack = glm::normalize(m_velocity) * m_punch_back;
		en->getVelocity() += punchBack; // add this kick
	}
	auto creature = dynamic_cast<Creature*>(entity);
	if (creature)
		creature->onHit(w, this, m_damage);
	markDead();
	return true;
}

bool EntityRoundBullet::onBlockHit(World& w, int blockX, int blockY)
{
	float numero = 8;
	float partNumero = 3.41459 * 2 / numero;
	for (int i = 0; i < numero; ++i)
	{
		if (rand() % 3 > 0)
			continue;
		float angle = i * partNumero + randDispersedFloat(0.2f);
		float partX = sin(angle);
		float partY = cos(angle);
		float truX = m_pos.x + partX;
		float truY = m_pos.y + partY;
		constexpr float radiusCheck = 0.8f;
		if (!BlockRegistry::get().getBlock(
			w.getBlockOrAir(m_pos.x + partX * radiusCheck, m_pos.y + partY * radiusCheck)->
			  block_id).hasCollisionBox())
			w.spawnParticle(ParticleList::bulletShatter, m_pos + glm::vec2(partX, partY) * 0.1f,
			                glm::vec2(partX, partY) * 0.15f, {0, -0.55f / 60}, 60, -1);
	}
	return Bullet::onBlockHit(w, blockX, blockY);
}

void EntityRoundBullet::onLoaded(World& w)
{
	w.getLightCalculator().registerLight(this);
}

void EntityRoundBullet::onUnloaded(World& w)
{
	w.getLightCalculator().removeLight(this);
}


Creature::Creature()
{
	m_flags |= EFLAG_COLLIDER;
	m_health_bar = Bar::buildDefault(glm::vec2(-1, -0.25f));
}

//Creature======================================================
void Creature::render(BatchRenderer2D& renderer)
{
	renderer.push(glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x, m_pos.y, 0)));
	renderer.submit(m_animation);

	m_health_bar.setValue(m_health / m_max_health);
	m_health_bar.setEnabled(m_health < m_max_health);
	m_health_bar.render(renderer);

	renderer.pop();

	if (m_bound.size() != 0 && Stats::show_collisionBox)
	{
		auto rect = m_bound.getBounds();
		auto mat = glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x + rect.x0, m_pos.y + rect.y0, 0));
		mat = glm::scale(mat, glm::vec3(rect.width(), rect.height(), 0));
		renderer.push(mat);
		renderer.submit(*Stats::bound_sprite);
		renderer.pop();
		if (getID() == ENTITY_TYPE_PLAYER)
		{
			/*auto& entityRectangle = m_bound.getBounds();
			for (int x = entityRectangle.x0-1; x < ceil(entityRectangle.x1)+1; ++x)
			{
				for (int y = entityRectangle.y0-1; y < ceil(entityRectangle.y1)+1; ++y)
				{
					Phys::Vect blockPos(x, y);
					blockPos += m_pos;

					auto stru = Stats::world->getLoadedBlockPointer((int)blockPos.x, (int)blockPos.y);
					if (stru == nullptr || stru->block_id == BLOCK_AIR)
						continue;
					auto& block = BlockRegistry::get().getBlock(stru->block_id);
					if (!block.hasCollisionBox())
						continue;
					auto blockBounds = block.getCollisionBox((int)blockPos.x, (int)blockPos.y, *stru).copy();

					blockBounds.plus({ x, y });
					if (Phys::isIntersects(blockBounds, m_bound))
					{
						ND_INFO("we have render intersction");
					}
				}
			}*/


			for (int y = -4; y < 4; ++y)
			{
				for (int x = -4; x < 4; ++x)
				{
					const BlockStruct* str = Stats::world->getBlock(x + m_pos.x, y + m_pos.y);
					if (str == nullptr)
						continue;
					auto& b = BlockRegistry::get().getBlock(str->block_id);
					if (!b.hasCollisionBox())
						continue;
					auto rect = b.getCollisionBox(x + m_pos.x, y + m_pos.y, *str).getBounds();
					auto mat = glm::translate(glm::mat4(1.0f),
					                          glm::vec3((int)m_pos.x + x + rect.x0, (int)m_pos.y + y + rect.y0, 0));
					mat = glm::scale(mat, glm::vec3(rect.width(), rect.height(), 0));
					renderer.push(mat);
					renderer.submit(*Stats::bound_sprite);
					renderer.pop();
				}
			}
		}
	}
}

void Creature::save(NBT& src)
{
	PhysEntity::save(src);
	src.set("Health", m_health);
}

void Creature::load(NBT& src)
{
	PhysEntity::load(src);
	m_health = src.get("Health", m_max_health);
}


//Sapling=======================================================
EntityType TileEntitySapling::getEntityType() const
{
	return ENTITY_TYPE_TILE_SAPLING;
}

// Torch=======================================================

void TileEntityTorch::update(World& w)
{
	m_tick_to_spawn_particle--;
	if (m_tick_to_spawn_particle == 0)
	{
		m_tick_to_spawn_particle = std::rand() % 60 + 2 * 60;
		w.spawnParticle(
			ParticleList::torch_fire,
			m_pos + glm::vec2(0.5f + randDispersedFloat(0.05f), 1),
			{randDispersedFloat(0.001), randFloat(0.001f) + 0.001f},
			vec2(0),
			60 * 3);

		w.spawnParticle(
			ParticleList::torch_smoke,
			m_pos + glm::vec2(0.5f + randDispersedFloat(0.05f), 1),
			{randDispersedFloat(0.01), randFloat(0.005f) + 0.01f},
			vec2(0),
			60 * 4);
	}
}

EntityType TileEntityTorch::getEntityType() const
{
	return ENTITY_TYPE_TILE_TORCH;
}

TileEntityChest::TileEntityChest()
{
	m_inventory.setInventorySize(27);
	m_inventory.setInventoryID("chest");
}

constexpr float maxOpenChestRadius = 10;

void TileEntityChest::onClicked(World& w, WorldEntity* entity)
{
	m_opener = entity->getID();
	auto& blok = getBlockStruct(w);
	auto& block = dynamic_cast<const BlockChest&>(BlockRegistry::get().getBlock(BLOCK_CHEST));

	auto e = w.getLoadedEntity(m_opener);
	if (e == nullptr || glm::distance2(e->getPosition(), m_pos) > maxOpenChestRadius * maxOpenChestRadius)
	{
		return; //ignore when player is far away
	}

	if (!block.isOpened(blok))
	{
		HUD::get()->registerGUIEntity(new GUIEntityChest(this));
		block.openChest(w, m_pos.x, m_pos.y, true);
	}
	else
	{
		HUD::get()->unregisterGUIEntity("chest");
		block.openChest(w, m_pos.x, m_pos.y, false);
	}
}

void TileEntityChest::update(World& w)
{
	//close chest if player is far away
	auto& blok = getBlockStruct(w);
	auto& block = dynamic_cast<const BlockChest&>(BlockRegistry::get().getBlock(BLOCK_CHEST));
	if (m_shouldClose)
		block.openChest(w, m_pos.x, m_pos.y, false);
	m_shouldClose = false;
	if (block.isOpened(blok) && m_opener != ENTITY_ID_INVALID)
	{
		auto e = w.getLoadedEntity(m_opener);
		if (e == nullptr || glm::distance2(e->getPosition(), m_pos) > maxOpenChestRadius * maxOpenChestRadius)
		{
			block.openChest(w, m_pos.x, m_pos.y, false);
			HUD::get()->unregisterGUIEntity("chest");
		}
	}
}

EntityType TileEntityChest::getEntityType() const
{
	return ENTITY_TYPE_TILE_CHEST;
}

void TileEntityChest::save(NBT& src)
{
	TileEntity::save(src);
	NBT t;
	m_inventory.save(t);
	src.set("inventory", t);
}

void TileEntityChest::load(NBT& src)
{
	TileEntity::load(src);
	m_inventory.load(src.get<NBT>("inventory"));
}

void TileEntityChest::onGUIEntityClosed()
{
	m_shouldClose = true;
}


void TileEntitySapling::update(World& w)
{
	TileEntity::update(w);

	if (m_age.hours() > 1)
	{
		m_age = 0;
		TreeGen::buildTree(w, getX(), getY());
	}
}


//TntEntity======================================================

EntityTNT::EntityTNT()
{

	m_timeToBoom = 60 * 2;
	m_blinkTime = 0;
	m_velocity = vec2(0);
	m_acceleration = {0.f, -9.8f / 60};
	m_max_velocity = vec2(50.0f / 60);

	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/player.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 4, 4);
	m_animation = Animation(&res);
	m_animation.setSpriteIndex(0, 0);

	m_animation.setPosition(glm::vec3(-1, 0, 0));
	m_animation.setSize(glm::vec2(2, 2));

	m_bound = Phys::toPolygon({-1, 0, 1, 2});
}


void EntityTNT::update(World& w)
{
	PhysEntity::computePhysics(w);

	++m_blinkTime;
	if (m_blinkTime > 12 * 1)
	{
		m_blinkTime = 0;
		flip = !flip;
		m_animation.setSpriteIndex(flip, 0);
	}

	if (m_timeToBoom-- == 0)
		boom(w);
}

void EntityTNT::boom(World& w)
{
	//auto point = w.getBlock(m_pos.x, m_pos.y);
	//if (point)
	//	w.setWall(m_pos.x, m_pos.y, 0);//leave the wall be

	//w.beginBlockSet();
	for (int i = -7; i < 7; ++i)
	{
		for (int j = -7; j < 7; ++j)
		{
			if (Phys::Vect(i, j).lengthCab() < 6)
			{
				if (auto point = w.getBlock(m_pos.x + i, m_pos.y + j))
					if (point && !point->isAir())
						w.setBlockWithNotify(m_pos.x + i, m_pos.y + j, 0);
			}
		}
	}
	for (int i = 0; i < 100; ++i)
	{
		auto angle = i / 50.f * 3.14159f;
		glm::vec2 vel(std::cos(angle), std::sin(angle));
		vel *= (std::rand() % 100) / 100.f / 3;

		w.spawnParticle(ParticleList::torch_smoke, m_pos, vel, -vel * 0.01f, 50 + std::rand() % 30);
		if (std::rand() % 10 == 0)
			w.spawnParticle(ParticleList::torch_fire, m_pos, vel * 0.1f, -vel * 0.001f, 50 + std::rand() % 30);
	}
	/*for (int i = 0; i < 20; ++i)
	{
		glm::vec2 speed(std::cos(6.28f * (float)i / 20), std::sin(6.28f * (float)i / 20));
		w.spawnParticle(ParticleList::torch_fire, m_pos, speed * 0.015f, speed * -0.01f, 200);

	}*/
	//w.flushBlockSet();
	markDead();
}

EntityType EntityTNT::getEntityType() const
{
	return ENTITY_TYPE_TNT;
}

void EntityTNT::save(NBT& src)
{
	Creature::save(src);
}

void EntityTNT::load(NBT& src)
{
	Creature::load(src);
}


//ZombieEntity======================================================


EntityZombie::EntityZombie()
{
	setMaxHealth(15);

	m_flags |= EFLAG_COLLIDER;
	m_velocity = vec2(0);
	m_acceleration = {0.f, -9.8f / 60};
	m_max_velocity = vec2(50.f / 60);

	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/zombie.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 16, 1);
	m_animation = Animation(&res, {0, 1, 2, 1, 0, 4, 3, 4});
	m_animation.setSpriteIndex(0, 2);
	m_animation.setPosition(glm::vec3(-1, 0, 0));
	m_animation.setSize(glm::vec2(2, 4));

	m_bound = Phys::toPolygon(Phys::Rectangle::createFromDimensions(-0.75f, 0, 1.5f, 2.9f));
}

void EntityZombie::update(World& w)
{
	auto lastPos = m_pos;
	PhysEntity::computePhysics(w);

	if (!m_found_player)
	{
		if (!m_tracer.isRunning())
		{
			auto& enties = w.getLoadedEntities();
			for (auto e : enties)
			{
				auto pointer = w.getLoadedEntity(e);
				if (pointer->getEntityType() == ENTITY_TYPE_PLAYER)
				{
					if (glm::distance2(pointer->getPosition(), m_pos) < std::pow(25, 2))
					{
						auto TPS = App::get().getTPS();
						m_tracer.init(&w, getID(),
						              {4.f / TPS, -9.f / TPS},
						              {10.f / TPS, 50.f / TPS});
						m_tracer.setTarget(e);
						break;
					}
				}
			}
		}
		else
		{
			auto response = m_tracer.update();
			if (response == TaskResponse::SUCCESS)
			{
				markDead();
				return;
			}
		}
	}
	if (this->m_pos.x > lastPos.x)
	{
		m_animation.setHorizontalFlip(false);
		m_animation_var = 1;
	}
	else if (this->m_pos.x < lastPos.x)
	{
		m_animation.setHorizontalFlip(true);
		m_animation_var = 1;
	}
	else
	{
		if (m_animation_var == 1)
			m_animation.setSpriteFrame(1, 0);
	}
	constexpr float animationBlocksPerFrame = 0.5f;
	m_pose += abs(lastPos.x - m_pos.x);
	if (m_pose - m_last_pose > animationBlocksPerFrame)
	{
		m_pose = m_last_pose;
		m_animation.nextFrame();
	}
}

EntityType EntityZombie::getEntityType() const
{
	return ENTITY_TYPE_ZOMBIE;
}

void EntityZombie::save(NBT& src)
{
	Creature::save(src);
}

void EntityZombie::load(NBT& src)
{
	Creature::load(src);
}

EntitySnowman::EntitySnowman()
{
	setMaxHealth(15);

	m_velocity = vec2(0);
	m_acceleration = {0.f, -9.8f / 60};
	m_max_velocity = vec2(50.f / 60);

	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/snowman.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 8, 1);
	m_animation = Animation(&res, {2, 3, 4, 3, 2, 5, 6, 5});
	m_animation.setSpriteIndex(0, 2);
	m_animation.setPosition(glm::vec3(-2, 0, 0));
	m_animation.setSize(glm::vec2(4, 5));

	m_bound = Phys::toPolygon(Phys::Rectangle::createFromDimensions(-0.95f, 0, 1.90f, 4.8f));
}

void EntitySnowman::update(World& w)
{
	auto lastPos = m_pos;
	PhysEntity::computePhysics(w);

	if (!m_found_player)
	{
		if (!m_tracer.isRunning())
		{
			auto& enties = w.getLoadedEntities();
			for (auto e : enties)
			{
				auto pointer = w.getLoadedEntity(e);
				if (pointer->getEntityType() == ENTITY_TYPE_PLAYER)
				{
					if (glm::distance2(pointer->getPosition(), m_pos) < std::pow(25, 2))
					{
						auto TPS = App::get().getTPS();
						m_tracer.init(&w, getID(),
						              {4.f / TPS, -9.f / TPS},
						              {10.f / TPS, 50.f / TPS});
						m_tracer.setTarget(e);
						break;
					}
				}
			}
		}
		else
		{
			auto response = m_tracer.update();
			if (response == TaskResponse::SUCCESS)
			{
				markDead();
				return;
			}
		}
	}
	if (this->m_pos.x > lastPos.x)
	{
		m_animation.setHorizontalFlip(false);
		m_animation_var = 1;
	}
	else if (this->m_pos.x < lastPos.x)
	{
		m_animation.setHorizontalFlip(true);
		m_animation_var = 1;
	}
	else
	{
		if (m_animation_var == 1)
			m_animation.setSpriteFrame(0, 0);
	}
	constexpr float animationBlocksPerFrame = 0.5f;
	m_pose += abs(lastPos.x - m_pos.x);
	if (m_pose - m_last_pose > animationBlocksPerFrame)
	{
		m_pose = m_last_pose;
		m_animation.nextFrame();
	}
}

EntityType EntitySnowman::getEntityType() const
{
	return ENTITY_TYPE_SNOWMAN;
}

void EntitySnowman::save(NBT& src)
{
	Creature::save(src);
}

void EntitySnowman::load(NBT& src)
{
	Creature::load(src);
}
