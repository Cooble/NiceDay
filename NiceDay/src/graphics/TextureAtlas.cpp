﻿#include "ndpch.h"
#include "TextureAtlas.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include <filesystem>

//should print everything
#define VERBOSE_TEXTURE_ATLAS_B 1

class Imager2D
{
	struct Pixel
	{
		uint8_t r, g, b, a;
	};

	inline static bool s_flipY = false;
public:
	static void flipY(bool flip) { s_flipY = flip; }

	static void copySubImage(
		void* src, void* dst,
		int srcW, int srcH, int dstW, int dstH,
		int cutX, int cutY, int cutW, int cutH,
		int pasteX, int pasteY
	)
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
};

struct Icon : Phys::Rectanglei
{
	int targetX, targetY;
	std::string filePath;
	std::string subName;

	inline int getSize() const
	{
		return width()*height();
		return std::max(width(), height());
	}
};

class Sorter
{
public:
	static void sort(Icon** array, size_t length)
	{
		quickSort(array, 0, length - 1);
	}

	template <typename T>
	static void invert(T* array, size_t length)
	{
		for (int i = 0; i < length / 2; ++i)
		{
			auto& first = array[i];
			auto& second = array[length - i - 1];

			auto temp = first;
			first = second;
			second = temp;
		}
	}

private:
	static void quickSort(Icon** array, int lowerIndex, int higherIndex)
	{
		int i = lowerIndex;
		int j = higherIndex;
		// calculate pivot number, I am taking pivot as middle index number
		Icon* pivot = array[lowerIndex + (higherIndex - lowerIndex) / 2];
		auto pivotSize = pivot->getSize();
		// Divide into two arrays
		while (i <= j)
		{
			/**
			 * In each iteration, we will identify a number from left side which
			 * is greater then the pivot value, and also we will identify a number
			 * from right side which is less then the pivot value. Once the search
			 * is done, then we exchange both numbers.
			 */
			while (array[i]->getSize() < pivotSize)
			{
				i++;
			}
			while (array[j]->getSize() > pivotSize)
			{
				j--;
			}
			if (i <= j)
			{
				exchangeNumbers(array, i, j);
				//move index to next position on both sides
				i++;
				j--;
			}
		}
		// call quickSort() method recursively
		if (lowerIndex < j)
			quickSort(array, lowerIndex, j);
		if (i < higherIndex)
			quickSort(array, i, higherIndex);
	}

	static void exchangeNumbers(Icon** array, int i, int j)
	{
		auto temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
};

struct Nod
{
	Icon* IMAGE;
	Phys::Rectanglei frame;
	Nod *n1, *n2;

	Nod(int x, int y, int width, int height, Icon* image = nullptr)
		: IMAGE(image),
		  frame(Phys::Rectanglei::createFromDimensions(x, y, width, height)),
		  n1(nullptr),
		  n2(nullptr)
	{
		if (IMAGE != nullptr)
		{
			IMAGE->targetX = frame.x0;
			IMAGE->targetY = frame.y0;
		}
	}

	~Nod()
	{
		if (n1)
			delete n1;
		if (n2)
			delete n2;
	}

	// return false if it cannot fit, true if it was put in
	bool fit(Icon* IMAGE)
	{
		int width = IMAGE->width();
		int height = IMAGE->height();

		if (width > frame.width() || height > frame.height())
			return false;
		Nod* down = new Nod(frame.x0, frame.y0, frame.width(), height);
		Nod* up = new Nod(frame.x0, frame.y0 + height, frame.width(), frame.height() - height);
		n1 = down;
		n2 = up;
		Nod* left = new Nod(frame.x0, frame.y0, width, height, IMAGE);
		Nod* right = new Nod(frame.x0 + width, frame.y0, frame.width() - width, height);
		down->n1 = left;
		down->n2 = right;
		return true;
	}

	inline bool isOccupied()
	{
		return n1 != nullptr;
	}

	inline bool hasImage()
	{
		return IMAGE != nullptr;
	}

	inline bool isDead()
	{
		return frame.width() < 1 || frame.height() < 1;
	}
};

static bool endsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}
static bool fit(Icon* icon, Nod* nod)
{
	if (!nod->isDead() && !nod->hasImage())
	{
		if (nod->isOccupied())
		{
			if (fit(icon, nod->n1))
				return true;
			else return fit(icon, nod->n2);
		}
		else
		{
			return nod->fit(icon);
		}
	}
	return false;
}

inline static std::string removeSuffix(const std::string& s)
{
	if(s.find_last_of('.')!=s.npos)
		return s.substr(0, s.find_last_of('.'));
	return s;

}
inline static std::string convertToShrunkFilePath(const std::string& folder, const std::string& currentFilePath)
{
	std::string out = currentFilePath.substr(folder.size());
	out = removeSuffix(out);
	std::replace(out.begin(), out.end(), '\\', '/');
	return out;
}

void TextureAtlas::createAtlas(const std::string& folder, int segmentCount, int segmentSize)
{
	ND_TRACE("[TextureAtlas] Creating...");


	int size = segmentSize * segmentCount;

	std::unordered_map<std::string, Icon> subImages;

	std::vector<std::string> allPNGPaths;

	defaultable_map_other<std::string, std::vector<std::string>> subImagesPath;

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (const auto& dirEntry : recursive_directory_iterator(folder))
	{
		if (!std::filesystem::is_regular_file(dirEntry))
			continue;
		std::string txtFile = dirEntry.path().string();
		if (endsWith(txtFile, ".txt"))
		{
			std::string pngFile = removeSuffix(txtFile) + ".png";

			//ND_INFO("this is txt file {}", current_file_path);

			std::string line;
			std::ifstream myfile(txtFile);
			bool labelSection = false;
			if (!myfile.is_open())
			{
				ND_WARN("Cannot open file: {}", txtFile);
				continue;
			}
			int labelCount = 0;
			Icon icon = {};
			while (getline(myfile, line))
			{
				//line to lowercase
				std::for_each(line.begin(), line.end(), [](char & c) {
					c = ::tolower(c);
				});

				if (line.empty())
					continue;;
				if (startsWith(line, "#"))
				{
					labelSection = true;
					continue;
				}
				if (startsWith(line, "/*"))
					break;
				if (labelSection)
				{
					int commaIndex = line.find_first_of(':', 0);
					if (commaIndex == std::string::npos)
					{
						continue;
					}
					std::string labelName = line.substr(0, commaIndex);
					std::string dime = line.substr(commaIndex + 1);

					std::string result;
					std::remove_copy(dime.begin(), dime.end(), std::back_inserter(result), ' ');
					dime = "";
					std::remove_copy(result.begin(), result.end(), std::back_inserter(dime), '\t');

					int offset = 0;
					int dims[4];

					dims[2] = 1; //default value
					dims[3] = 1;

					for (int i = 0; i < 4; ++i)
					{
						size_t endOffset = dime.find_first_of(',', offset);
						if (endOffset != std::string::npos)
							dims[i] = std::stoi(dime.substr(offset, endOffset));
						else
						{
							dims[i] = std::stoi(dime.substr(offset));
							break;
						}
						offset = endOffset + 1;
					}
					icon.filePath = convertToShrunkFilePath(folder, txtFile);

					icon.subName = labelName;
					icon.x0 = dims[0];

					icon.y0 = dims[1];
					icon.setWidth(dims[2]);
					icon.setHeight(dims[3]);
					subImages[icon.filePath + icon.subName] = icon;
					subImagesPath[pngFile].push_back(icon.filePath + icon.subName);
					icon = {};
					++labelCount;
				}
			}
			
			myfile.close();
		}
		else if (endsWith(txtFile, ".png"))
		{
			allPNGPaths.push_back(txtFile);
		}
	}
	for (auto& currentPNGPath : allPNGPaths) //get those which dont have txt file but only png
	{
		if (endsWith(currentPNGPath, "atlas.png"))
			continue;;
		if (!subImagesPath.contains(currentPNGPath))
		{
			//ND_INFO("Missing txt file {}", currentPNGPath);
			int width = 0;
			int height = 0;
			int BPP = 0;


			void* currentImage = stbi_load(currentPNGPath.c_str(), &width, &height, &BPP, 4);
			ASSERT(currentImage, "invalid image");
			stbi_image_free(currentImage);


			Icon icon;
			icon.x0 = 0;
			icon.y0 = 0;
			icon.setWidth(width / segmentSize);
			icon.setHeight(height / segmentSize);
			icon.subName = "main";
			icon.filePath = convertToShrunkFilePath(folder, currentPNGPath);
			subImages[icon.filePath + icon.subName] = icon;

			subImagesPath[currentPNGPath].push_back(icon.filePath + icon.subName);
		}
	}


	
	Icon** ordered_icons = new Icon*[subImages.size()];

	int index = 0;
	for (auto& i : subImages)
		ordered_icons[index++] = &i.second;


	Sorter::sort(ordered_icons, subImages.size());
	Sorter::invert(ordered_icons, subImages.size());

	Nod n(0, 0, segmentCount, segmentCount);

	for (int i = 0; i < subImages.size(); ++i)
	{
#if VERBOSE_TEXTURE_ATLAS_B
		ND_TRACE("[TextureAtlas] Loading subtexture: {}", (ordered_icons[i]->filePath + +":\t\t" + ordered_icons[i]->subName));
#endif
		if (!fit(ordered_icons[i], &n))
		{
			ND_ERROR("TextureAtlas: {} size[{}], too small", folder, size);
			return;
		}
	}

	void* atlas = malloc(4 * size * size);
	memset(atlas, 0, 4 * size * size);


	Imager2D::flipY(true);
	for (auto& path : subImagesPath)
	{
		int width = 0;
		int height = 0;
		int BPP = 0;


		void* currentImage = stbi_load(path.first.c_str(), &width, &height, &BPP, 4);
		ASSERT(currentImage,"invalid image");

		for (auto& value : path.second)
		{
			Icon& icon = subImages[value];
			Imager2D::copySubImage(
				currentImage, atlas,
				width, height, size, size,
				icon.x0 * segmentSize, icon.y0 * segmentSize, 
				icon.width() * segmentSize, icon.height() * segmentSize,
				icon.targetX * segmentSize, icon.targetY * segmentSize);
			m_subtextures[value] = half_int(icon.targetX, icon.targetY);
		}

		stbi_image_free(currentImage);
	}

	stbi_write_png((folder + "atlas.png").c_str(), size, size, STBI_rgb_alpha, atlas, size * 4);
	ND_TRACE("[TextureAtlas] Done: {}", folder + "atlas.png");
}
