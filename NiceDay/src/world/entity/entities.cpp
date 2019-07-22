#include "ndpch.h"
#include "entities.h"
#include "graphics/BatchRenderer2D.h"
#include "entity_datas.h"
#include "graphics/Texture.h"
#include "world/block/BlockRegistry.h"
#include "world/World.h"
#include "world/block/block_datas.h"


//PhysEntity======================================================


//return if we have intersection
inline static bool findIntersection(World* w, const Phys::Vect& entityPos, const Phys::Polygon& entityBound)
{
	auto& entityRectangle = entityBound.getBounds();
	for (int i = entityRectangle.x0; i < entityRectangle.x1; ++i)
	{
		for (int j = entityRectangle.y0; j < entityRectangle.y1; ++j)
		{
			Phys::Vect blockPos(i, j);
			blockPos += entityPos;

			auto stru = w->getLoadedBlockPointer((int)blockPos.x, (int)blockPos.y);
			if (stru == nullptr || stru->block_id == BLOCK_AIR)
				continue;
			auto& block = BlockRegistry::get().getBlock(stru->block_id);
			if (!block.hasBounds())
				continue;
			auto blockBounds = block.getBounds((int)blockPos.x, (int)blockPos.y, *stru);

			blockBounds.plus({i, j});
			if (Phys::isIntersects(blockBounds, entityBound))
				return true;
		}
	}
	return false;
}

static float signum(float f)
{
	//todo this function sucks performance-wisely
	if (f == 0)
		return 0;
	return (abs(f) / f);
}

void PhysEntity::computePhysics(World* w)
{
	//motion=========================================
	m_velocity += m_acceleration;
	m_velocity = Phys::clamp(m_velocity, -m_max_velocity, m_max_velocity);

	//wind resistance================================
	constexpr float windResistance = 0.01;
	constexpr float windVelocityDepenceFactor = 1000000;//we dont need higher air resistance the bigger the velocity
	constexpr float floorResistance = 0.1;//negative numbers mean conveyor belt Yeah!

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

	//collision detection============================
	auto possibleEntityPos = m_pos + m_velocity;

	//check if we can procceed in x, y direction
	if (findIntersection(w, possibleEntityPos, m_bound))
	{
		possibleEntityPos = m_pos;
		possibleEntityPos.x += m_velocity.x;
		//check if we can procceed at least in x direction
		if (findIntersection(w, possibleEntityPos, m_bound))
		{
			possibleEntityPos = m_pos;
			possibleEntityPos.y += m_velocity.y;
			//check if we can procceed at least in y direction
			if (findIntersection(w, possibleEntityPos, m_bound))
			{
				//we have nowhere to go
				possibleEntityPos = m_pos;
				m_velocity = 0;
			}
			else
				m_velocity.x = 0;
		}
		else
		{
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
	m_velocity = src.get("velocity", Phys::Vect(0, 0));
	m_acceleration = src.get("acceleration", Phys::Vect(0, -9.8f / 60));
}

//Creature======================================================
void Creature::render(BatchRenderer2D& renderer)
{
	renderer.push(glm::translate(glm::mat4(1.0f), glm::vec3(m_pos.x, m_pos.y, 0)));
	renderer.submit(m_sprite);
	renderer.pop();
}

void Creature::save(NBT& src)
{
	PhysEntity::save(src);
}

void Creature::load(NBT& src)
{
	PhysEntity::load(src);
}


//PlayerEntity======================================================


EntityPlayer::EntityPlayer()
{
	m_velocity = 0.0f;
	m_acceleration = {0.f, -9.8f / 60};
	m_max_velocity = {50.f / 60};

	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/player.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 4, 4);
	m_sprite = Sprite(&res);
	m_sprite.setSpriteIndex(0, 3);
	m_sprite.setPosition(glm::vec3(-2, 0, 0));
	m_sprite.setSize(glm::vec2(4, 5));

	m_bound = Phys::toPolygon({-0.5f, 0, 0.5f, 3});
}

void EntityPlayer::update(World* w)
{
	auto lastPos = m_pos;
	PhysEntity::computePhysics(w);

	if (this->m_pos.x > lastPos.x)
	{
		m_pose = 1;
	}
	else if (this->m_pos.x < lastPos.x)
	{
		m_pose = 2;
	}
	else
	{
		if (this->m_pos.y != lastPos.y)
			m_pose = 0;
	}
	m_sprite.setSpriteIndex(m_pose, 3);
}


EntityType EntityPlayer::getEntityType() const
{
	return ENTITY_TYPE_PLAYER;
}

void EntityPlayer::save(NBT& src)
{
	Creature::save(src);
	src.set("playerName", m_name);
}

void EntityPlayer::load(NBT& src)
{
	Creature::load(src);
	m_name = src.get<std::string>("playerName", "Karel");
}


//TntEntity======================================================

EntityTNT::EntityTNT()
{
	m_timeToBoom = 60 * 2;
	m_blinkTime = 0;
	m_velocity = 0.0f;
	m_acceleration = {0.f, -9.8f / 60};
	m_max_velocity = 50.0f / 60;

	static SpriteSheetResource res(Texture::create(
		                               TextureInfo("res/images/player.png")
		                               .filterMode(TextureFilterMode::NEAREST)
		                               .format(TextureFormat::RGBA)), 4, 4);
	m_sprite = Sprite(&res);
	m_sprite.setSpriteIndex(0, 0);

	m_sprite.setPosition(glm::vec3(-1, 0, 0));
	m_sprite.setSize(glm::vec2(2, 2));

	m_bound = Phys::toPolygon({-1, 0, 1, 2});
}

void EntityTNT::update(World* w)
{
	PhysEntity::computePhysics(w);

	++m_blinkTime;
	if (m_blinkTime > 12 * 1)
	{
		m_blinkTime = 0;
		flip = !flip;
		m_sprite.setSpriteIndex(flip, 0);
	}

	if (m_timeToBoom-- == 0)
	{
		auto point = w->getLoadedBlockPointer(m_pos.x, m_pos.y);
		if(point)
			w->setWall(m_pos.x, m_pos.y, 0);
		
		w->beginBlockSet();
		for (int i = -7; i < 7; ++i)
		{
			for (int j = -7; j < 7; ++j)
			{
				if (Phys::Vect(i, j).lengthCab() < 6)
				{
					if (auto point = w->getLoadedBlockPointer(m_pos.x + i, m_pos.y + j))
						if (point && !point->isAir())
							w->setBlock(m_pos.x + i, m_pos.y + j, 0);
				}
			}
		}
		w->flushBlockSet();
		w->killEntity(getID());
	}
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
	m_velocity = 0.0f;
	m_acceleration = { 0.f, -9.8f / 60 };
	m_max_velocity = { 10.f / 60 };

	static SpriteSheetResource res(Texture::create(
		TextureInfo("res/images/player.png")
		.filterMode(TextureFilterMode::NEAREST)
		.format(TextureFormat::RGBA)), 4, 4);
	m_sprite = Sprite(&res);
	m_sprite.setSpriteIndex(0, 2);
	m_sprite.setPosition(glm::vec3(-2, 0, 0));
	m_sprite.setSize(glm::vec2(4, 5));

	m_bound = Phys::toPolygon({ -0.5f, 0, 0.5f, 3 });
}

void EntityZombie::update(World* w)
{
	if (!m_found_player) {
		if (!m_tracer.isRunning()) {
			auto& enties = w->getLoadedEntities();
			for (auto e : enties){
				auto pointer = w->getLoadedEntity(e);
				if (pointer->getEntityType() == ENTITY_TYPE_PLAYER)	{
					if (pointer->getPosition().distanceSquared(m_pos) < std::pow(25, 2)) {
						m_tracer.init(w, getID());
						m_tracer.setTarget(e);
						break;
					}
				}
			}
		}
		else
		{
			auto response = m_tracer.update();
			if (response==TaskResponse::SUCCESS)
			{
				m_found_player = true;
				ND_INFO("zombie approached platyer");
				m_velocity.x = 0;
				m_velocity.y = 5;//lets fly away:D
				m_acceleration.x = 0;
				m_acceleration.y = 10.f / 60;
			}
		}
	}
	auto lastPos = m_pos;
	PhysEntity::computePhysics(w);

	if (this->m_pos.x > lastPos.x)
	{
		m_pose = 1;
	}
	else if (this->m_pos.x < lastPos.x)
	{
		m_pose = 2;
	}
	else
	{
		if (this->m_pos.y != lastPos.y)
			m_pose = 0;
	}
	m_sprite.setSpriteIndex(m_pose, 2);
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

