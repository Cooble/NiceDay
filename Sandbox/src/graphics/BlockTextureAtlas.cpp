#include "ndpch.h"
#include "BlockTextureAtlas.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include "files/FUtil.h"

#include <filesystem>
#include "world/ChunkMesh.h"
#include "graphics/Imager2D.h"

//should print everything
#define VERBOSE_TEXTURE_ATLAS 0
using namespace nd;
struct Icon : Phys::Rectanglei
{
	int targetX, targetY;
	std::string filePath;
	std::string subName;
	bool hascorners=false;

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
static std::string edgeString = "EDGE";

static bool endsWith(const std::string& str, const std::string& suffix)
{
	return str.size() >= suffix.size() && 0 == str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

static bool startsWith(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

static void removeEDGE(std::string& e)
{
	if(endsWith(e,edgeString))
		e = e.substr(0, e.size() - edgeString.size());
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

void BlockTextureAtlas::createAtlas(const std::string& folder, int segmentCount, int segmentSize)
{
	FUTIL_ASSERT_EXIST(folder);

	stbi_flip_vertically_on_write(false);
	stbi_set_flip_vertically_on_load(false);
	
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
			bool corner = false;
			Icon icon = {};
			while (getline(myfile, line))
			{
				//line to lowercase
				std::for_each(line.begin(), line.end(), [](char & c) {
					c = ::tolower(c);
				});

				if (line.empty())
					continue;;
				if(!labelSection&&startsWith(line,"edge:"))
				{
					std::string yes = line.substr(std::string("edge:").size());
					std::string result;
					std::remove_copy(yes.begin(), yes.end(), std::back_inserter(result), ' ');
					yes = "";
					std::remove_copy(result.begin(), result.end(), std::back_inserter(yes), '\t');
					if (startsWith(yes, "1") || startsWith(yes, "true")) {
						icon.hascorners = true;
						corner = true;
					}
				}
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
					removeEDGE(icon.filePath);

					icon.subName = labelName;
					icon.x0 = dims[0];

					icon.y0 = dims[1];
					icon.setWidth(dims[2]);
					icon.setHeight(dims[3]);
					subImages[icon.filePath + icon.subName] = icon;
					if (!std::filesystem::exists(pngFile)) {
						pngFile = removeSuffix(pngFile);
						pngFile += "EDGE.png";
					}
					subImagesPath[pngFile].push_back(icon.filePath + icon.subName);
					icon = {};
					++labelCount;
				}
			}
			
			myfile.close();

			if(labelCount==0&&corner)//has corners but no specified main label
			{
				int width = 0;
				int height = 0;
				int BPP = 0;


				void* currentImage = stbi_load(pngFile.c_str(), &width, &height, &BPP, 4);
				ASSERT(currentImage, "invalid image: {}",pngFile.c_str());
				stbi_image_free(currentImage);

				Icon icon = {};
				icon.filePath = convertToShrunkFilePath(folder, txtFile);

				icon.hascorners = true;
				icon.subName = "main";
				icon.x0 = width/segmentSize/EDGE_COLOR_TRANSFORMATION_FACTOR;
				icon.y0 = height/segmentSize/EDGE_COLOR_TRANSFORMATION_FACTOR;
				icon.setWidth(width / segmentSize-icon.x0);
				icon.setHeight(height / segmentSize - icon.y0);

				subImages[icon.filePath + icon.subName] = icon;

				subImagesPath[pngFile].push_back(icon.filePath + icon.subName);
			}
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
			ASSERT(currentImage, "invalid image: {}",currentPNGPath.c_str());
			stbi_image_free(currentImage);


			Icon icon;
			icon.x0 = 0;
			icon.y0 = 0;
			icon.setWidth(width / segmentSize);
			icon.setHeight(height / segmentSize);
			icon.subName = "main";
			icon.filePath = convertToShrunkFilePath(folder, currentPNGPath);
			
			if(endsWith(icon.filePath, edgeString))
			{
				removeEDGE(icon.filePath);
				icon.hascorners = true;
				icon.x0 = width / segmentSize / EDGE_COLOR_TRANSFORMATION_FACTOR;
				icon.y0 = height / segmentSize / EDGE_COLOR_TRANSFORMATION_FACTOR;
				icon.setWidth(width / segmentSize - icon.x0);
				icon.setHeight(height / segmentSize - icon.y0);
			}
			subImages[icon.filePath + icon.subName] = icon;

			subImagesPath[currentPNGPath].push_back(icon.filePath + icon.subName);
		}
	}


	bool hasCorners = false;
	for (auto& i : subImages)
	{
		if (i.second.hascorners)
			hasCorners = true;
	}
	Icon cornerIcon;
	if (hasCorners)
	{
		cornerIcon.targetX = 0;
		cornerIcon.targetY = 0;
		cornerIcon.setWidth(segmentCount / EDGE_COLOR_TRANSFORMATION_FACTOR);
		cornerIcon.setHeight(segmentCount / EDGE_COLOR_TRANSFORMATION_FACTOR);
	}

	Icon** ordered_icons = new Icon*[subImages.size()];

	int index = 0;
	for (auto& i : subImages)
		ordered_icons[index++] = &i.second;


	Sorter::sort(ordered_icons, subImages.size());
	Sorter::invert(ordered_icons, subImages.size());

	Nod n(0, 0, segmentCount, segmentCount);
	if (hasCorners)
	{
		if (!fit(&cornerIcon, &n))
		{
			ND_ERROR("TextureAtlas: {} size[{}], too small", folder, size);
			return;
		}
	}

	for (int i = 0; i < subImages.size(); ++i)
	{
#if VERBOSE_TEXTURE_ATLAS
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
	int indexxx = 0;
	for (auto& path : subImagesPath)
	{
		int width = 0;
		int height = 0;
		int BPP = 0;


		void* currentImage = stbi_load(path.first.c_str(), &width, &height, &BPP, 4);
		ASSERT(currentImage,"invalid image: {}",path.first.c_str());

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
			if (icon.hascorners)
			{
				float multi = (float)segmentSize / EDGE_COLOR_TRANSFORMATION_FACTOR;
				Imager2D::copySubImage(
					currentImage, atlas,
					width, height, size, size,
					icon.x0 * multi-2,
					icon.y0 * multi-2,
					icon.width() *  multi,
					icon.height()* multi,
					icon.targetX *  multi,
					icon.targetY *  multi);
			}
		}

		stbi_image_free(currentImage);
	}

	stbi_write_png((folder + "atlas.png").c_str(), size, size, STBI_rgb_alpha, atlas, size * 4);
	free(atlas);
	ND_TRACE("[TextureAtlas] Done: {}", folder + "atlas.png");
}

half_int BlockTextureAtlas::getTexture(const std::string& fileName, const std::string& subName) const
{
	std::string subNameC = subName;
	std::transform(subNameC.begin(), subNameC.end(), subNameC.begin(),
	               [](unsigned char c) { return std::tolower(c); });

	auto& i = m_subtextures.find(fileName + subNameC);
	if (i == m_subtextures.end())
	{
		ND_ERROR("Trying to retrieve not loaded texture: {}", (fileName + +":\t\t"+ subNameC));
		return 0;
	}
	return i->second;
}
