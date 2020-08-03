#include "core/App.h"
#include "testEntt.h"
#include "entt/entt.hpp"
#include <imguifiledialog/ImGuiFileDialog.h>

struct TagComponent
{
	char name[30];

	TagComponent(const char* c) { set(c); }
	TagComponent(const TagComponent& e)
	{
		set(e.name);
	}

	void set(const char* c)
	{
		int size = strlen(c);
		if(size>29)
			return;
		memcpy(&name, c, size+1);
	}
	const char* operator()()
	{
		return name;
	}
};
struct TransformComponent
{
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;
	
};

class Scen;

class Entity
{
private:
	entt::entity m_entity=entt::null;
	entt::registry* m_reg=nullptr;
	Entity(entt::entity e,entt::registry* r):m_entity(e),m_reg(r){}
	friend Scen;
public:
	Entity(const Entity& e) = default;
	Entity() = default;
	template <typename ComponentType, typename... Args>
	void emplaceOrReplace(Args&&... args)
	{
		m_reg->emplace_or_replace<ComponentType>(m_entity, std::forward<Args>(args)...);
	}
	template <typename... ComponentTypes>
	bool any() const 
	{
		return m_reg->any<ComponentTypes>(m_entity);
	}
	template <typename... ComponentTypes>
	bool has() const 
	{
		return m_reg->has<ComponentTypes>(m_entity);
	}
	template <typename... ComponentTypes>
	void remove()
	{
		m_reg->remove<ComponentTypes>(m_entity);
	}
	template <typename... ComponentTypes>
	void removeIfExists()
	{
		m_reg->remove_if_exists<ComponentTypes>(m_entity);
	}

	void removeAll()
	{
		m_reg->remove_all(m_entity);
	}
	template <typename... ComponentTypes>
	decltype(auto) get()
	{
		return m_reg->get<ComponentTypes>(m_entity);
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

class Scen
{
private:
	entt::registry m_reg;
public:
	Entity createEntity() { return { m_reg.create(), &m_reg }; }
	template<typename... Component,typename... Exclude>
	decltype(auto) view(entt::exclude_t<Exclude...> = {})
	{
		return m_reg.view<Component...>(entt::exclude_t<Exclude...>());
	}
	template<typename... Owned,typename... Exclude>
	decltype(auto) group(entt::exclude_t<Exclude...> = {})
	{
		return m_reg.group<Owned...>(entt::exclude_t<Exclude...>());
	}
	entt::registry& reg() { return m_reg; }
};
Scen scen;
Entity e;
void TestEnntLayer::onAttach()
{

	{
		e = scen.createEntity();
		e.emplaceOrReplace<TagComponent>("never");
		e.emplaceOrReplace<TransformComponent>(glm::vec3(0), glm::vec3(0), glm::vec3(0));
	}
	{
		e = scen.createEntity();
		e.emplaceOrReplace<TagComponent>("runnning");
		//e.emplaceOrReplace<TransformComponent>(glm::vec3(0), glm::vec3(0), glm::vec3(0));

	}
	{
		e = scen.createEntity();
		e.emplaceOrReplace<TagComponent>("tonever");
		e.emplaceOrReplace<TransformComponent>(glm::vec3(0), glm::vec3(0), glm::vec3(0));
	}

	
}

void TestEnntLayer::onDetach()
{
}

void TestEnntLayer::onUpdate()
{
}

void TestEnntLayer::onRender()
{

}

void TestEnntLayer::onImGuiRender()
{		bool t = true;
		if(ImGui::Begin("Karel"))
		{
			auto group = scen.group<TagComponent>(entt::exclude_t<TransformComponent>());
			for (auto entity : group)
			{
				auto& tag = group.get<TagComponent>(entity);
				ImGui::Text(tag.operator()());
			}
		
		}
		ImGui::End();
}
