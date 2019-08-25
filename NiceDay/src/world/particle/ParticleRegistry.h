#pragma once

class TextureAtlas;
typedef int ParticleID;
class ParticleRegistry
{
public:
	static inline ParticleRegistry& get() {
		static ParticleRegistry s_instance;
		return s_instance;
	}
	struct ParticleTemplate
	{
		std::string name;
		half_int texture_pos;
		half_int size;
		int frameCount;
		int ticksPerFrame;
		float rotationSpeed;

		ParticleTemplate(const std::string& name,half_int size,int frameCount,int ticksPerFrame, float rotationSpeed):
		name(name),size(size),frameCount(frameCount),ticksPerFrame(ticksPerFrame), rotationSpeed(rotationSpeed)
		{}

	};
private:
	std::vector<ParticleTemplate> m_templates;
	ParticleRegistry()=default;
public:
	ParticleRegistry(ParticleRegistry const&) = delete;
	void operator=(ParticleRegistry const&) = delete;

	ParticleID registerTemplate(const ParticleTemplate& t);

	void initTextures(const TextureAtlas& atlas);

	inline const ParticleTemplate& getTemplate(ParticleID id) const
	{
		ASSERT(id >= 0 && id < m_templates.size(), "invalid ParticleID");
		return m_templates[id];
	}
};

