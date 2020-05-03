#pragma once

namespace nd_registry
{
	// is world remote or textures are needed as well
	void registerEverything(bool loadTextures);

	void registerItemBlocks();
	void registerItems();
	void registerBlocks();
	void registerEntities();
	void registerBiomes();
	void registerParticles();

	void loadTexturesBlocks();
	void loadTexturesItems();
	void loadTexturesEntities();
	void loadTexturesParticles();

}
