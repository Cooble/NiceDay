#pragma once
namespace nd {

class Imager2D
{
	struct Pixel
	{
		uint8_t r, g, b, a;
	};

	inline static bool s_flipY = false;
public:
	inline static void flipY(bool flip) { s_flipY = flip; }

	static void copySubImage(
		void* src, void* dst,
		int srcW, int srcH, int dstW, int dstH,
		int cutX, int cutY, int cutW, int cutH,
		int pasteX, int pasteY
	);
};
}
