#include "ndpch.h"
#include "TextureAtlas.h"
#include "Imager2D.h"
#include "core/physShapes.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include <filesystem>
#include "graphics/API/Texture.h"
#include "files/FUtil.h"

//should print everything
#define VERBOSE_TEXTURE_ATLAS_B 0



struct Icon : Phys::Rectanglei
{
	int targetX, targetY;
	std::string filePath;
	std::string subName;
	void* data;

	int getSize() const
	{
		return width() * height();
		return std::max(width(), height());
	}
};

class Sorter
{
public:
	static void sort(Icon** array, size_t length)
	{
		if (length == 1)
			return;
		quickSort(array, 0, length - 1);
	}

	template <typename T>
	static void invert(T* array, size_t length)
	{
		if (length == 1)
			return;
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
	Nod* n1, * n2;

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


static bool fit(Icon* icon, Nod* nod)
{
	if (!nod->isDead() && !nod->hasImage())
	{
		if (nod->isOccupied())
		{
			if (fit(icon, nod->n1))
				return true;
			return fit(icon, nod->n2);
		}
		return nod->fit(icon);
	}
	return false;
}


static std::string convertToShrunkFilePath(std::string_view folder, std::string_view currentFilePath)
{
	std::string out = std::string(currentFilePath.substr(folder.size()+1));
	FUtil::removeSuffix(out);
	FUtil::cleanPathString(out);
	return out;
}

TextureAtlas::~TextureAtlas()
{
	if (m_texture)
		delete m_texture;
}

bool TextureAtlas::createAtlas(std::string_view folder, int segmentCount, int segmentSize, TextureAtlasFlags flags)
{
	stbi_flip_vertically_on_write(false);
	stbi_set_flip_vertically_on_load(flags & TextureAtlasFlags_FlipY);

	m_segmentCount = segmentCount;
	ND_TRACE("[TextureAtlas] Creating...");

	std::unordered_map<std::string, Icon> subImages;

	std::vector<std::string> allPNGPaths;

	defaultable_map_other<std::string, std::vector<std::string>> subImagesPath;

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (const auto& dirEntry : recursive_directory_iterator(ND_RESLOC(folder)))
	{
		if (!std::filesystem::is_regular_file(dirEntry))
			continue;
		std::string file = dirEntry.path().string();
		if (SUtil::endsWith(file, ".txt"))
		{
			std::string pngFile = FUtil::removeSuffixConst(file) + ".png";

			//ND_INFO("this is txt file {}", current_file_path);

			std::string line;
			std::ifstream myfile(file);
			bool labelSection = false;
			if (!myfile.is_open())
			{
				ND_WARN("Cannot open file: {}", file);
				continue;
			}
			int labelCount = 0;
			Icon icon = {};
			while (getline(myfile, line))
			{
				//line to lowercase
				SUtil::toLower(line);

				if (line.empty())
					continue;
				if (SUtil::startsWith(line, '#'))
				{
					labelSection = true;
					continue;
				}
				if (SUtil::startsWith(line, "/*"))
					break;
				if (labelSection)
				{
					int commaIndex = line.find_first_of(':', 0);
					if (commaIndex == std::string::npos)
						continue;
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
					icon.filePath = convertToShrunkFilePath(ND_RESLOC(folder), file);

					icon.subName = labelName;
					icon.x0 = dims[0];

					icon.y0 = dims[1];
					icon.setWidth(dims[2]);
					icon.setHeight(dims[3]);
					subImages[icon.filePath +'/'+ icon.subName] = icon;
					subImagesPath[pngFile].push_back(icon.filePath + '/' + icon.subName);
					icon = {};
					++labelCount;
				}
			}

			myfile.close();
		}
		else if (SUtil::endsWith(file, ".png")&&!SUtil::endsWith(file, "atlas.png"))
		{
			allPNGPaths.push_back(file);
		}
	}
	for (auto& currentPNGPath : allPNGPaths) //get those which dont have txt file but only png
	{
		if (!subImagesPath.contains(currentPNGPath))
		{
			//ND_INFO("Missing txt file {}", currentPNGPath);
			int width = 0;
			int height = 0;
			int BPP = 0;

			void* currentImage = stbi_load(currentPNGPath.c_str(), &width, &height, &BPP, 4);
			ASSERT(currentImage, "invalid image: {}", currentPNGPath.c_str());
			stbi_image_free(currentImage);

			//SANITY DEPLETED!
			//
			//interesting note: if the icon is left just like: Icon icon;
			//there is a corruption on the stack
			//if its declared like this: Icon icon={};
			//its fine
			//Only god knows the reason....
			Icon icon = {};
			icon.x0 = 0;
			icon.y0 = 0;
			icon.setWidth(width / segmentSize);
			icon.setHeight(height / segmentSize);
			icon.subName = "main";
			icon.filePath = convertToShrunkFilePath(ND_RESLOC(folder), currentPNGPath);
			auto name = icon.filePath + '/' + icon.subName;
			subImages[name] = icon;

			subImagesPath[currentPNGPath].push_back(name);
		}
	}


	Icon** ordered_icons = new Icon * [subImages.size()];

	int index = 0;
	for (auto& i : subImages)
		ordered_icons[index++] = &(i.second);


	Sorter::sort(ordered_icons, subImages.size());
	Sorter::invert(ordered_icons, subImages.size());

	int size = segmentSize * m_segmentCount;
	
	while (true)
	{
		bool cont = false;
		Nod n(0, 0, m_segmentCount, m_segmentCount);
		for (int i = 0; i < subImages.size(); ++i)
		{
#if VERBOSE_TEXTURE_ATLAS_B
			ND_TRACE("[TextureAtlas] Loading subtexture: {}", (ordered_icons[i]->filePath + +":\t\t" + ordered_icons[i]->subName));
#endif
			if (!fit(ordered_icons[i], &n))
			{
				if (flags & TextureAtlasFlags_DontResizeIfNeccessary) {
					ND_ERROR("TextureAtlas: {} size[{}], too small (consider disabling TextureAtlasUVFlags_DontResizeIfNeccessary)", folder, size);
				
					delete[] ordered_icons;
					return false;
				}
				m_segmentCount++;
				size = m_segmentCount * segmentSize;
				cont = true;
				break;
			}
		}
		if (!cont)
			break;
	}

	void* atlas = malloc(4 * size * size);
	ZeroMemory(atlas, 4 * size * size);


	Imager2D::flipY(true);
	for (auto& path : subImagesPath)
	{
		int width = 0;
		int height = 0;
		int BPP = 0;

		
		void* currentImage = stbi_load(path.first.c_str(), &width, &height, &BPP, 4);
		ASSERT(currentImage, "invalid image: {}", path.first.c_str());

		for (auto& value : path.second)
		{
			Icon& icon = subImages[value];
			Imager2D::copySubImage(
				currentImage, atlas,
				width, height, size, size,
				icon.x0 * segmentSize, icon.y0 * segmentSize,
				icon.width() * segmentSize, icon.height() * segmentSize,
				icon.targetX * segmentSize, icon.targetY * segmentSize);
			m_subtextures[SID(value)] = half_int(icon.targetX, icon.targetY);
		}

		stbi_image_free(currentImage);
	}
	if (!(flags & TextureAtlasFlags_DontCreateTexture))
		m_texture = Texture::create(TextureInfo().size(size, size).format(TextureFormat::RGBA).pixels(atlas));
	if (flags & TextureAtlasFlags_CreateFile)
		stbi_write_png((ND_RESLOC(folder)+ "atlas.png").c_str(), size, size, STBI_rgb_alpha, atlas, size * 4);
	free(atlas);
	ND_TRACE("[TextureAtlas] Done: {}", std::string(folder)+ "atlas.png");
	return true;
}
half_int TextureAtlas::getSubImage(const std::string& fileName, const char* subName) const
{
	std::string subNameC = subName;
	SUtil::toLower(subNameC);
	subNameC = fileName + '/' + subNameC;
	auto& i = m_subtextures.find(SID(subNameC));
	if (i == m_subtextures.end())
	{
		ND_ERROR("Trying to retrieve not loaded texture: {}", subNameC);
		return 0;
	}
	return i->second;
}

TextureAtlasUV::~TextureAtlasUV()
{
	if (m_texture)
		delete m_texture;

}

bool TextureAtlasUV::createAtlas(std::string_view folder, int size,int padding,TextureAtlasFlags flags)
{
	m_size = size;
	stbi_flip_vertically_on_write(false);
	stbi_set_flip_vertically_on_load(flags & TextureAtlasFlags_FlipY);

	ND_TRACE("[TextureAtlas] Creating...");

	std::vector<Icon> icons;

	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	for (const auto& dirEntry : recursive_directory_iterator(ND_RESLOC(folder)))
	{
		if (!std::filesystem::is_regular_file(dirEntry))
			continue;
		std::string file = dirEntry.path().string();
		if (SUtil::endsWith(file, ".png") && !SUtil::endsWith(file, "atlas.png"))
		{
			int width = 0;
			int height = 0;
			int BPP = 0;

			void* currentImage = stbi_load(file.c_str(), &width, &height, &BPP, 4);
			ASSERT(currentImage, "invalid image: {}", file.c_str());
			//stbi_image_free(currentImage);

			//SANITY DEPLETED!
			//
			//interesting note: if the icon is left just like: Icon icon;
			//there is a corruption on the stack
			//if its declared like this: Icon icon={};
			//its fine
			//Only god knows the reason....
			Icon icon = {};
			icon.x0 = 0;
			icon.y0 = 0;
			icon.x1 = width+padding*2;
			icon.y1 = height+padding*2;
			icon.data = currentImage;
			icon.filePath = convertToShrunkFilePath(ND_RESLOC(folder), file);
			icons.push_back(icon);
		}
		
	}

	Icon** ordered_icons = new Icon * [icons.size()];

	int index = 0;
	for (auto& i : icons)
		ordered_icons[index++] = &i;

	Sorter::sort(ordered_icons, icons.size());
	Sorter::invert(ordered_icons, icons.size());


	while(true)
	{
		bool cont = false;
		Nod n(0, 0, m_size, m_size);
		for (int i = 0; i < icons.size(); ++i)
		{
#if VERBOSE_TEXTURE_ATLAS_B
			ND_TRACE("[TextureAtlas] Loading subtexture: {}", (ordered_icons[i]->filePath + +":\t\t" + ordered_icons[i]->subName));
#endif
			if (!fit(ordered_icons[i], &n))
			{
				if (flags & TextureAtlasFlags_DontResizeIfNeccessary) {
					ND_ERROR("TextureAtlas: {} size[{}], too small (consider disabling TextureAtlasUVFlags_DontResizeIfNeccessary)", folder, m_size);
					for (auto& icon : icons)
						stbi_image_free(icon.data);
					delete[] ordered_icons;
					return false;
				}
				m_size *= 1.5f;
				cont = true;
				break;
			}
		}
		if (!cont)
			break;
	}
	

	void* atlas = malloc(4 * m_size * m_size);
	memset(atlas, 0, 4 * m_size * m_size);


	Imager2D::flipY(false);
	for (auto& icon : icons)
	{
		Imager2D::copySubImage(
			icon.data, atlas,
			icon.width() - padding * 2, icon.height() - padding * 2,
			m_size, m_size,
			0,0,
			icon.width() - padding * 2, icon.height() - padding * 2,
			icon.targetX+padding, icon.targetY+padding);
		stbi_image_free(icon.data);
		auto& subTex = m_subtextures[SID(icon.filePath)];
		subTex.min = glm::vec2(icon.targetX+padding, icon.targetY+padding) / (float)m_size;
		subTex.max = glm::vec2(icon.targetX+icon.width()-padding, icon.targetY+icon.height()-padding) / (float)m_size;
		subTex.pixelSize = { icon.width() - padding * 2, icon.height() - padding * 2};
	}
	if(!(flags&TextureAtlasFlags_DontCreateTexture))
		m_texture = Texture::create(TextureInfo().size(m_size, m_size).format(TextureFormat::RGBA).pixels(atlas));
	if(flags & TextureAtlasFlags_CreateFile)
		stbi_write_png((std::string(ND_RESLOC(folder))+ "/atlas.png").c_str(), m_size, m_size, STBI_rgb_alpha, atlas, m_size * 4);
	free(atlas);
	ND_TRACE("[TextureAtlas] Done: {}", folder);
	return true;
}
