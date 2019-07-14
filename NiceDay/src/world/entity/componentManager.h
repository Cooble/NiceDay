#pragma once
#include "EntityManager.h"
#include "graphics/Sprite.h"

/*
typedef uint32_t PhysicsInstance;

class PhysicsComponentManager
{
private:
	struct PhysicsComponentStruct
	{
		Entity* entity;
		glm::vec2* pos;
		glm::vec2* velocity;
		glm::vec2* acceleration;
		size_t n;

		PhysicsComponentStruct(size_t n):n(n)
		{
			size_t byteSize = n * (sizeof(Entity) + 3 * sizeof(glm::vec2));
			void* buff = malloc(byteSize);
			entity = (Entity*)buff;
			pos = (glm::vec2*)(entity + n);
			velocity = (glm::vec2*)(pos + n);
			acceleration = (glm::vec2*)(velocity + n);
		}
		PhysicsComponentStruct(const PhysicsComponentStruct& old,size_t newSize)
			:PhysicsComponentStruct(newSize)
		{
			memcpy(entity, old.entity, sizeof(Entity)*old.n);
			memcpy(pos, old.pos, sizeof(glm::vec2)*old.n);
			memcpy(velocity, old.velocity, sizeof(glm::vec2)*old.n);
			memcpy(acceleration, old.acceleration, sizeof(glm::vec2)*old.n);
		}

		~PhysicsComponentStruct()
		{
			if (entity) {
				free(entity);
				entity = nullptr;
			}
		}
	};

private:
	size_t m_max_size;
	size_t m_current_size;
	PhysicsComponentStruct m_data;
	std::unordered_map<Entity, PhysicsInstance> m_map;

private:
	void resize(size_t newSize);

public:
	PhysicsComponentManager(size_t maxSize);

	inline size_t getCurrentSize() const { return m_current_size; }
	PhysicsInstance getInstance(Entity e);
	void removeInstance(Entity e);

	inline glm::vec2& position(PhysicsInstance i) { return m_data.pos[i]; }
	inline glm::vec2& velocity(PhysicsInstance i) { return m_data.velocity[i]; }
	inline glm::vec2& acceleration(PhysicsInstance i) { return m_data.acceleration[i]; }
	inline Entity entity(PhysicsInstance i) { return m_data.entity[i]; }

};


typedef uint32_t MeshInstance;

class MeshComponentManager
{
private:
	struct MeshComponentStruct
	{
		Entity* entity;
		Sprite* sprite;
		
		size_t n;

		MeshComponentStruct(size_t n) :n(n)
		{
			size_t byteSize = n * (sizeof(Entity) + 3 * sizeof(Sprite));
			void* buff = malloc(byteSize);
			entity = (Entity*)buff;
			sprite = (Sprite*)(entity + n);
			
		}
		MeshComponentStruct(const MeshComponentStruct& old, size_t newSize)
			:MeshComponentStruct(newSize)
		{
			memcpy(entity, old.entity, sizeof(Entity)*old.n);
			memcpy(sprite, old.sprite, sizeof(Sprite)*old.n);
			
		}

		~MeshComponentStruct()
		{
			if (entity) {
				free(entity);
				entity = nullptr;
			}
		}
	};

private:
	size_t m_max_size;
	size_t m_current_size;
	MeshComponentStruct m_data;
	std::unordered_map<Entity, MeshInstance> m_map;

private:
	void resize(size_t newSize);
public:
	MeshComponentManager(size_t maxSize);

	PhysicsInstance getInstance(Entity e);
	void removeInstance(Entity e);
	inline size_t getCurrentSize() const { return m_current_size; }

	inline Sprite& mesh(MeshInstance i) { return m_data.sprite[i]; }
	inline Entity entity(MeshInstance i) { return m_data.entity[i]; }
};
*/