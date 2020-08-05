#pragma once
#include <entt/entt.hpp>
#include "components.h"

class NewScene;

//entt::entity wrapper/handle
class Entity
{
private:
	entt::entity m_entity = entt::null;
	entt::registry* m_reg = nullptr;
	Entity(entt::entity e, entt::registry* r) :m_entity(e), m_reg(r) {}
public:
	Entity() = default;
	friend NewScene;
	template <typename ComponentType, typename... Args>
	decltype(auto) emplaceOrReplace(Args&&... args)
	{
		return m_reg->emplace_or_replace<ComponentType>(m_entity, std::forward<Args>(args)...);
	}
	template <typename... ComponentTypes>
	bool any() const
	{
		return m_reg->any<ComponentTypes...>(m_entity);
	}
	template <typename... ComponentTypes>
	bool has() const
	{
		return m_reg->has<ComponentTypes...>(m_entity);
	}
	template <typename... ComponentTypes>
	void remove()
	{
		m_reg->remove<ComponentTypes...>(m_entity);
	}
	template <typename... ComponentTypes>
	void removeIfExists()
	{
		m_reg->remove_if_exists<ComponentTypes...>(m_entity);
	}

	void removeAll()
	{
		m_reg->remove_all(m_entity);
	}
	template <typename... ComponentTypes>
	decltype(auto) get()
	{
		if constexpr (sizeof...(ComponentTypes) == 1) {
			return  m_reg->get<ComponentTypes...>(m_entity);
		}
		else {
			return std::forward_as_tuple(m_reg->get<ComponentTypes...>(m_entity));
		}
	}

	void destroy()
	{
		return m_reg->destroy(m_entity);
	}

	bool valid() const
	{
		return m_reg->valid(m_entity);
	}
};


class NewScene
{
private:
	entt::registry m_reg;
public:
	Entity createEntity(const char* name="Invalid") {
		Entity e{m_reg.create(), &m_reg };
		e.emplaceOrReplace<TagComponent>(name);
		return e;
	}
	Entity wrap(entt::entity e) { return { e, &m_reg }; }
	template<typename... Component, typename... Exclude>
	decltype(auto) view(entt::exclude_t<Exclude...> = {})
	{
		return m_reg.view<Component...>(entt::exclude_t<Exclude...>());
	}
	template<typename... Owned, typename... Exclude>
	decltype(auto) group(entt::exclude_t<Exclude...> = {})
	{
		return m_reg.group<Owned...>(entt::exclude_t<Exclude...>());
	}
	entt::registry& reg() { return m_reg; }
};
