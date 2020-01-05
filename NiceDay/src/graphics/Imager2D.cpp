#include "ndpch.h"
#include "Imager2D.h"

void Imager2D::copySubImage(void* src, void* dst, int srcW, int srcH, int dstW, int dstH, int cutX, int cutY, int cutW,
                            int cutH, int pasteX, int pasteY)
{
	if (s_flipY)
	{
		pasteY = dstH - pasteY - cutH;
		cutY = srcH - cutY - cutH;
	}
	auto src_buff = (Pixel*)src;
	auto dst_buff = (Pixel*)dst;

	Pixel* srcPixel = src_buff + (cutY * srcW + cutX);
	Pixel* dstPixel = dst_buff + (pasteY * dstW + pasteX);

	for (int y = 0; y < cutH; ++y)
	{
		memcpy(dstPixel + (y * dstW), srcPixel + (y * srcW), cutW * sizeof(Pixel));
	}
}
