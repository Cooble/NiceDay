#include "ndpch.h"
#include "BiomeForest.h"
#include "core/App.h"
#include "world/WorldRenderManager.h"

static const int forestLayersCount = 3;
static const int forestCloudsCount = 4;
static const int forestSunCount = 1;
static const int forestMoonCount = 1;
static const int forestStarCount = 1;
static const int forestLayerIdx = 0;
static const int forestCloudIdx = forestLayersCount;
static const int forestSkySize = forestStarCount +forestMoonCount + forestSunCount;
static const int forestSize =forestLayersCount+forestCloudsCount;

struct cloudLocs
{
	glm::vec2 pos;
	float speed;
};

static cloudLocs* cloudLocations = nullptr;

static void initClouds()
{
	if (!cloudLocations)
	{
		cloudLocations = new cloudLocs[forestCloudsCount];
		ZeroMemory(cloudLocations, forestCloudsCount * sizeof(cloudLocs));
		for (int i = 0; i < forestCloudsCount; ++i)
		{
			cloudLocations[i].speed = 2;
		}
	}
}

BiomeForest::BiomeForest()
	: Biome(BIOME_FOREST)
{
	m_background_light = 16;
	m_sprites_size = forestSize;
	m_sky_sprites_size = forestSkySize;
	m_normal_lighting_enable = true;
	m_sprites = new Sprite2D*[m_sprites_size];
	m_sky_sprites = new Sprite2D*[m_sky_sprites_size];
	initClouds();

	int index = 0;

	TextureInfo info = TextureInfo("res/images/bg/sun.png").wrapMode(TextureWrapMode::CLAMP_TO_BORDER);
	m_sun = new Sprite2D(Texture::create(info));

	info.file_path = "res/images/bg/moon.png";
	m_moon = new Sprite2D(Texture::create(info));

	info.file_path = "res/images/bg/stars.png";
	info.wrapMode(TextureWrapMode::REPEAT);

	m_star = new Sprite2D(Texture::create(info));
	m_star->setPosition(glm::vec2(-1, -1));
	m_star->setScale(glm::vec2(2, 2));

	

	m_sky_sprites[index++] = m_star;
	m_sky_sprites[index++] = m_sun;
	m_sky_sprites[index++] = m_moon;
	
	index = 0;
	for (int i = 0; i < forestLayersCount; i++)
	{
		TextureInfo info(std::string("res/images/bg/forest_") + std::to_string(i) + ".png");
		info.wrap_mode_s = TextureWrapMode::REPEAT;
		info.wrap_mode_t = TextureWrapMode::CLAMP_TO_BORDER;
		m_sprites[index] = new Sprite2D(Texture::create(info));
		m_sprites[index]->setPosition(glm::vec2(-1, -1));
		m_sprites[index]->setScale(glm::vec2(2, 2));
		index++;
	}
	
	for (int i = 0; i < forestCloudsCount; i++)
	{
		TextureInfo info(std::string("res/images/bg/cloud_") + std::to_string(i % 3) + ".png");
		//info.wrap_mode_s = TextureWrapMode::REPEAT;
		//info.wrap_mode_t = TextureWrapMode::REPEAT;
		info.wrapMode(TextureWrapMode::CLAMP_TO_BORDER);
		m_sprites[index] = new Sprite2D(Texture::create(info));
		//m_sprites[index]->setPosition(glm::vec2(-1, -1));
		//m_sprites[index]->setScale(glm::vec2(2, 2));
		index++;
	}


	
}

void BiomeForest::updateSprites(World* m_world, Camera* m_camera)
{
	using namespace glm;
	vec2 screenDim = vec2(App::get().getWindow()->getWidth(), App::get().getWindow()->getHeight());
	vec2 lowerScreen = m_camera->getPosition() - ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	vec2 upperScreen = m_camera->getPosition() + ((screenDim / (float)BLOCK_PIXEL_SIZE) / 2.0f);
	screenDim = upperScreen - lowerScreen;

	//todo background images should be in worldrenderer drawn at full hd and not scaled down by 2
	//todo clouds someday upon the stars wake up where the clouds are falling behind...
	for (int b = forestLayerIdx; b < forestLayerIdx+forestLayersCount; b++)
	{
		Sprite2D& s = *m_sprites[b];
		int i = b - forestLayerIdx;

		auto texDim = vec2(s.getTexture().getWidth() / 1.5, s.getTexture().getHeight() / 1.5);
		auto pos = vec2(
			m_world->getInfo().chunk_width / 2 * WORLD_CHUNK_SIZE,
			-i * 2 + (float)s.getTexture().getHeight() / BLOCK_PIXEL_SIZE / 3 + m_world->getInfo().terrain_level);
		//pos = pos - (m_camera->getPosition()-pos);
		vec2 meshLower = pos;
		vec2 meshUpper = pos + texDim / (float)BLOCK_PIXEL_SIZE;
		vec2 meshDim = meshUpper - meshLower;

		vec2 delta = m_camera->getPosition() - (meshLower + meshUpper) / 2.0f; //delta of centers
		delta = delta / (3.0f - i);
		delta.y /= 2.0f; //we need smaller change in y

		auto transl = delta / meshDim;

		if (i == 2)
		{
			transl.y += 0.5f; //blocks lower
		}
		auto scal = screenDim / meshDim;
		mat4 t(1.0f);
		t = glm::translate(t, vec3(transl.x, transl.y, i / 1000.f));
		t = glm::scale(t, vec3(scal.x, scal.y, 1));
		s.setUVMatrix(t);
	}
	//stars resize
	{
		auto& s = *m_star;
		auto texDim = vec2(s.getTexture().getWidth(), s.getTexture().getHeight());
		auto screenDim = App::get().getWindow()->getDimensions();
		auto scale = screenDim / texDim *0.5f;
		auto tran = screenDim / 2.f / texDim;
		mat4 t(1.0f);
		t = glm::translate(t, vec3(tran.x, tran.y, 0 / 1000.f));
		t = glm::scale(t, vec3(scale.x, scale.y, 1));
		s.setUVMatrix(t);
	}
	//clouds
	for (int i = forestCloudIdx; i < forestCloudIdx+forestCloudsCount; i++)
	{
		auto cloudId = i - forestCloudIdx;
		Sprite2D& s = *m_sprites[i];

		auto texDim = vec2(s.getTexture().getWidth(), s.getTexture().getHeight());
		auto displayDim = App::get().getWindow()->getDimensions();
		auto texDisRation = texDim / displayDim;

		auto& cloudPos = cloudLocations[cloudId].pos;
		cloudPos.x += cloudLocations[cloudId].speed*m_world->m_time_speed;
		if (cloudPos.x > 1)
		{
			cloudPos.x = -1.5;
			cloudPos.y = 0.9 + (std::rand() % 64) / 64.f;
			cloudLocations[cloudId].speed = std::rand() % 64 / 64.f * 0.001f + 0.0005f;
		}
		auto deltaY = m_camera->getPosition().y - (m_world->getInfo().terrain_level - 32);

		mat4 t(1.0f);
		//t = glm::translate(t, vec3(0, 0, i / 1000.f));
		t = glm::translate(t, vec3(cloudPos.x, cloudPos.y - deltaY / 100, (cloudPos.y - deltaY / 100)/100));
		t = glm::scale(t, vec3(texDisRation.x * 2, texDisRation.y * 2, 1));
		s.setModelMatrix(t);


		s.setUVMatrix(glm::mat4(1.f));
	}


	//============sun moon===========================================================
	{
		auto time = m_world->getWorldTime();
		float hour = time.hour();

		constexpr float startSun = 6;
		constexpr float sunDelay = 12;
		constexpr float stopSun = startSun + sunDelay;

		constexpr float startMoon = 19;
		constexpr float moonDelay = 10;
		constexpr float stopMoon = 5;
		float sunPhase = (hour - startSun) / (stopSun - startSun);
		float moonPhase;

		bool isDay = hour >= startSun && hour <= stopSun;
		bool isNight = hour >= startMoon || hour <= stopMoon;

		if (hour >= startMoon)
		{
			moonPhase = (hour - startMoon) / (moonDelay);
		}
		else
		{
			moonPhase = (24 - startMoon + hour) / (moonDelay);
		}
		//show stars
		float starVisibility = min((-abs(moonPhase - 0.5f) + 0.5f) * 5,1.f);
		m_star->setAlpha(isNight ? starVisibility : 0);

		{
			//sun
			float rat = 0.1;//todo this ratio sucks bad scaling when changing window dims
			//moon
			auto texDim = vec2(m_sun->getTexture().getWidth(), m_sun->getTexture().getHeight());
			auto displayDim = App::get().getWindow()->getDimensions();
			//texDim=
			auto texDisRation = 1.f / glm::normalize(displayDim);

			mat4 t(1.f);
			t = glm::scale(t, vec3(texDisRation.x, texDisRation.y, 1));//view


			float cutAngle = 0.8;
			t = glm::translate(t, glm::vec3(0, -1, 0));//down
			t = glm::rotate(t, -sunPhase * (3.14159f - 2 * cutAngle) + 3.14159f - cutAngle, { 0,0,1 });//rot
			t = glm::translate(t, isDay ? glm::vec3(1.3, 0, 0) : glm::vec3(100, 0, 0));//tra1 rameno
			t = glm::scale(t, vec3(rat, rat, 1));//sca to down
			t = glm::rotate(t, -3.14159f / 2, { 0,0,1 });//rot image
			t = glm::translate(t, glm::vec3(-0.5, -0.5, 0.1));//tra to center


			/*t = glm::translate(t,vec3(0, 1, 0));
			t = glm::scale(t, vec3(texDisRation.x * 2, texDisRation.y * 2, 1));
			t = glm::rotate(t, -moonPhase * 3.14159f + 3.14159f, {0, 0, 1});
			t = glm::translate(t,vec3(-0.5, -0.5, 0));*/

			//t = viewM * rotM * tra2M * scaM * traM;

			m_sun->setModelMatrix(t);
			m_sun->setUVMatrix(glm::mat4(1.f));
		}
		{
			float rat = 0.1;//todo this ratio sucks bad scaling when changing window dims
			//moon
			auto texDim = vec2(m_moon->getTexture().getWidth(), m_moon->getTexture().getHeight());
			auto displayDim = App::get().getWindow()->getDimensions();
			//texDim=
			auto texDisRation = 1.f / glm::normalize(displayDim);

			mat4 t(1.f);
			t = glm::scale(t, vec3(texDisRation.x, texDisRation.y, 1));//view


			float cutAngle = 0.8;
			t = glm::translate(t, glm::vec3(0, -1, 0));//down
			t = glm::rotate(t, -moonPhase * (3.14159f - 2 * cutAngle) + 3.14159f - cutAngle, { 0,0,1 });//rot
			t = glm::translate(t, isNight ? glm::vec3(1.3, 0, 0) : glm::vec3(100, 0, 0));//tra1 rameno
			t = glm::scale(t, vec3(rat, rat, 1));//sca to down
			t = glm::rotate(t, -3.14159f / 2, { 0,0,1 });//rot image
			t = glm::translate(t, glm::vec3(-0.5, -0.5, 0.1));//tra to center


			/*t = glm::translate(t,vec3(0, 1, 0));
			t = glm::scale(t, vec3(texDisRation.x * 2, texDisRation.y * 2, 1));
			t = glm::rotate(t, -moonPhase * 3.14159f + 3.14159f, {0, 0, 1});
			t = glm::translate(t,vec3(-0.5, -0.5, 0));*/

			//t = viewM * rotM * tra2M * scaM * traM;

			m_moon->setModelMatrix(t);
			m_moon->setUVMatrix(glm::mat4(1.f));
		}
	}
}
