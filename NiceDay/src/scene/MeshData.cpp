#include "MeshData.h"

#include "files/FUtil.h"

MeshData::MeshData(size_t maxVertexCount, size_t vertexSize, size_t maxIndexCount, const VertexBufferLayout& layout)
{
	allocate(maxVertexCount, vertexSize, maxIndexCount, layout);
}

MeshData::MeshData()
{
}

MeshData::~MeshData()
{
	if (m_vertices)
		free(m_vertices);
	if (m_indices)
		free(m_indices);
}

void MeshData::allocate(size_t maxVertexCount, size_t vertexSize, size_t maxIndexCount, const VertexBufferLayout& layout)
{
	if (m_vertices)
		free(m_vertices);
	if (m_indices)
		free(m_indices);


	m_indices = nullptr;
	m_vertex_count = maxVertexCount;
	m_index_count = maxIndexCount;
	m_vertex_count_max = maxVertexCount;
	m_index_count_max = maxIndexCount;
	m_vertex_size = vertexSize;
	m_layout = layout;
	m_vertices = (char*)malloc(vertexSize * maxVertexCount);
	if (maxIndexCount)
		m_indices = (char*)malloc(sizeof(uint32_t) * maxIndexCount);
}


namespace MeshDataFactory
{
	std::hash<std::string> hasher;

	struct BigVertex
	{
		glm::vec3 pos = { 0,0,0 };
		glm::vec3 norm = { 0,0,0 };
		glm::vec2 uv = { 0,0 };

		//the ultimate performance nightmare
		uint64_t hash() const
		{
			char death[(3 + 3 + 2) * sizeof(float)];
			float* p = (float*)death;
			*p++ = pos.x;
			*p++ = pos.y;
			*p++ = pos.z;

			*p++ = norm.x;
			*p++ = norm.y;
			*p++ = norm.z;

			*p++ = uv.x;
			*p++ = uv.y;

			//todo different hash method
			std::string ss(death, sizeof(death));

			return hasher(ss);
		}
	};

	constexpr int bufsize = 200000;
	/*static std::array<glm::vec3, bufsize> v_data;
	static std::array<glm::vec3, bufsize> vn_data;
	static std::array<glm::vec2, bufsize> vt_data;
	static std::array<BigVertex, bufsize> bigVertices;
	static std::array<int, bufsize * 4> indices_data;*/

    	static glm::vec3* v_data = new glm::vec3[bufsize];
	static glm::vec3* vn_data = new glm::vec3[bufsize];
	static glm::vec2* vt_data = new glm::vec2[bufsize];
	static BigVertex* bigVertices = new BigVertex[bufsize];
	static int* indices_data = new int[bufsize * 4];
	//todo change back after gh_actions

	//hash, index
	static std::unordered_map<uint64_t, uint64_t> bigVertexIndexes;

	static int charsIn(std::string s, char c)
	{
		int res = 0;
		for (int i = 0; i < s.length(); i++)
			if (s[i] == c)
				res++;
		return res;
	}

	MeshData* buildFromObj(const std::string& path, bool usePosNormUv)
	{
		FUTIL_ASSERT_EXIST(path);

		size_t vSize = 0;
		size_t vnSize = 0;
		size_t vtSize = 0;
		size_t bigSize = 0;
		size_t indSize = 0;

		ZeroMemory(&bigVertices, bufsize * sizeof(BigVertex));

		{
			TimerStaper t("fileLoad");
			std::ifstream infile(ND_RESLOC(path));
			std::string line;
			while (std::getline(infile, line))
			{
				std::istringstream iss(line);
				std::string type;
				iss >> type;

				// vertex xyzw
				if (type == "v")
				{
					double x = 0, y = 0, z = 0;
					iss >> x >> y >> z;
					v_data[vSize++] = { x, y, z };
				}
				// texture uv
				else if (type == "vt")
				{
					double u = 0, v = 0;
					iss >> u >> v;
					vt_data[vtSize++] = { u, v };
				}
				// normal
				else if (type == "vn")
				{
					double x = 0, y = 0, z = 0;
					iss >> x >> y >> z;
					vn_data[vnSize++] = { x, y, z };
				}
				// faces
				else if (type == "f")
				{
					int packIndex = 0;
					std::string pack;
					while (std::getline(iss, pack, ' '))
					{
						if (pack.empty())
							continue;
						BigVertex biggie;
						std::istringstream packStream(pack);

						uint32_t indicesIndex[3]{ 0, 0, 0 };
						// vertex=0, vt=1, vn=2
						size_t indexik = 0;
						std::string val;
						while (std::getline(packStream, val, '/'))
							if (!val.empty()) {
								indicesIndex[indexik++] = std::stoi(val) - 1;
							}
							else
							{
								indexik++;
							}
						biggie.pos = v_data[indicesIndex[0]];
						if (vtSize)
							biggie.uv = vt_data[indicesIndex[1]];
						if (vnSize)
							biggie.norm = vn_data[indicesIndex[2]];
						auto has = biggie.hash();
						uint32_t bigVIndex;
						auto it = bigVertexIndexes.find(biggie.hash());
						if (it == bigVertexIndexes.end())
						{
							bigVIndex = bigSize;
							bigVertexIndexes[has] = bigSize;
							bigVertices[bigSize++] = biggie;
						}
						else
						{
							bigVIndex = it->second;
						}
						//now we have bigIndex
						indices_data[indSize++] = bigVIndex;
						packIndex++;
					}
				}
			}
			infile.close();
			//now we have filled bigVertices and indices_data
		}
		auto model = new MeshData;

		VertexBufferLayout layout;


		layout.pushElement(g_typ::VEC3);
		if (vnSize || usePosNormUv)
			layout.pushElement(g_typ::VEC3);
		if (vtSize || usePosNormUv)
			layout.pushElement(g_typ::VEC2);

		auto bigvertexSize = layout.getStride();

		model->allocate(bigSize, bigvertexSize, indSize, layout);

		memcpy(model->getIndices(), (char*)&indices_data, indSize * sizeof(uint32_t));

		if (bigvertexSize == sizeof(BigVertex) || usePosNormUv)
		{
			//we have all (pos,normals,uvs)
			memcpy(model->getVertices(), (char*)&bigVertices, bigSize * sizeof(BigVertex));
		}
		else
		{
			TimerStaper t("recopying");
			auto pointer = (float*)model->getVertices();
			for (int i = 0; i < model->getVerticesCount(); ++i)
			{
				auto& bg = bigVertices[i];
				*pointer++ = bg.pos.x;
				*pointer++ = bg.pos.y;
				*pointer++ = bg.pos.z;
				//todo find out if memcpy is faster
				//memcpy(pointer, &bigVertices[i].pos, sizeof(glm::vec3));
				//pointer += 3;

				if (vnSize)
				{
					*pointer++ = bg.norm.x;
					*pointer++ = bg.norm.y;
					*pointer++ = bg.norm.z;

					//	memcpy(pointer, &bigVertices[i].norm, sizeof(glm::vec3));
					//	pointer += 3;
				}
				if (vtSize)
				{
					*pointer++ = bg.uv.x;
					*pointer++ = bg.uv.y;
					//	memcpy(pointer, &bigVertices[i].uv, sizeof(glm::vec2));
					//	pointer += 2;
				}
			}
		}

		model->setID(path.c_str());
		return model;
	}

	MeshData* buildWirePlane(int x, int z)
	{
		auto mesh = new MeshData;
		auto size = (x + z) * 2;

		VertexBufferLayout l{ g_typ::VEC3 };
		mesh->allocate(size, sizeof(glm::vec3), 0, l);

		auto point = (glm::vec3*) mesh->getVertices();

		for (int xx = 0; xx < x; ++xx)
		{
			*point++ = { xx,0,0 };
			*point++ = { xx,0,z };
		}
		for (int zz = 0; zz < z; ++zz)
		{
			*point++ = { 0,0,zz };
			*point++ = { x,0,zz };
		}
		mesh->m_topology = Topology::LINES;
		mesh->setID(("BuildWire" + std::to_string(64871 * x + z * 123456)).c_str());
		return mesh;
	}


	MeshData* buildCube(float scale)
	{
		/*static const float cubeVertices[] = {
	                -1.0f,-1.0f,-1.0f,
	                -1.0f,-1.0f, 1.0f,
	                -1.0f, 1.0f, 1.0f,
	                1.0f, 1.0f,-1.0f,
	                -1.0f,-1.0f,-1.0f,
	                -1.0f, 1.0f,-1.0f,
	                1.0f,-1.0f, 1.0f,
	                -1.0f,-1.0f,-1.0f,
	                1.0f,-1.0f,-1.0f,
	                1.0f, 1.0f,-1.0f,
	                1.0f,-1.0f,-1.0f,
	                -1.0f,-1.0f,-1.0f,
	                -1.0f,-1.0f,-1.0f,
	                -1.0f, 1.0f, 1.0f,
	                -1.0f, 1.0f,-1.0f,
	                1.0f,-1.0f, 1.0f,
	                -1.0f,-1.0f, 1.0f,
	                -1.0f,-1.0f,-1.0f,
	                -1.0f, 1.0f, 1.0f,
	                -1.0f,-1.0f, 1.0f,
	                1.0f,-1.0f, 1.0f,
	                1.0f, 1.0f, 1.0f,
	                1.0f,-1.0f,-1.0f,
	                1.0f, 1.0f,-1.0f,
	                1.0f,-1.0f,-1.0f,
	                1.0f, 1.0f, 1.0f,
	                1.0f,-1.0f, 1.0f,
	                1.0f, 1.0f, 1.0f,
	                1.0f, 1.0f,-1.0f,
	                -1.0f, 1.0f,-1.0f,
	                1.0f, 1.0f, 1.0f,
	                -1.0f, 1.0f,-1.0f,
	                -1.0f, 1.0f, 1.0f,
	                1.0f, 1.0f, 1.0f,
	                -1.0f, 1.0f, 1.0f,
	                1.0f,-1.0f, 1.0f
		};*/
		auto cubeVertices = new float[50 * 3];//todo change back to static allocation after done with gh_actions compilation


		MeshData* mesh = new MeshData;
		VertexBufferLayout l{ g_typ::VEC3 };
		mesh->allocate(36, sizeof(glm::vec3), 0, l);
		auto point = (glm::vec3*)mesh->getVertices();
		memcpy(point, (char*)cubeVertices, mesh->getVerticesSize());
		for (int i = 0; i < mesh->getVerticesCount(); ++i)
			*mesh->vertex<glm::vec3>(i) *= scale;
		mesh->setID(("BuildCube" + std::to_string(scale)).c_str());
		return mesh;
	}

	struct BigHead
	{
		size_t m_vertex_size;

		size_t m_vertex_count_max = 0;
		size_t m_index_count_max = 0;
		int elements = 0;
		MeshData::AABB box;
	};

	void writeBinaryFile(const std::string& filePath, MeshData& mesh)
	{
		BigHead h;
		h.m_vertex_size = mesh.getOneVertexSize();
		h.m_vertex_count_max = mesh.getVerticesCountMax();
		h.m_index_count_max = mesh.getIndicesCountMax();
		h.elements = mesh.getLayout().getElements().size();
		h.box = mesh.getAABB();

		FILE* file = fopen(ND_RESLOC(filePath).c_str(), "wb");

		fwrite(&h, sizeof(BigHead), 1, file);
		fwrite(&mesh.getLayout().getElements()[0], sizeof(VertexBufferElement), mesh.getLayout().getElements().size(), file);
		fwrite(mesh.getVertices(), mesh.getVerticesSize(), 1, file);
		if (mesh.getIndicesCountMax())
			fwrite(mesh.getIndices(), mesh.getIndicesSize(), 1, file);


		fclose(file);
	}

	MeshData* readBinaryFile(const std::string& filePath)
	{
		FUTIL_ASSERT_EXIST(filePath);

		FILE* file = fopen(ND_RESLOC(filePath).c_str(), "rb");
		if (!file)
			return nullptr;
		BigHead h;
		fread(&h, sizeof(BigHead), 1, file);

		VertexBufferLayout layout;
		for (int i = 0; i < h.elements; ++i)
		{
			VertexBufferElement e;
			fread(&e, sizeof(VertexBufferElement), 1, file);
			layout.pushElement(e);
		}
		MeshData& mesh = *new MeshData;
		mesh.allocate(h.m_vertex_count_max, h.m_vertex_size, h.m_index_count_max, layout);
		fread(mesh.getVertices(), mesh.getVerticesSize(), 1, file);
		if (mesh.getIndicesCountMax())
			fread(mesh.getIndices(), mesh.getIndicesSize(), 1, file);

		fclose(file);
		mesh.setID(filePath.c_str());
		mesh.setAABB(h.box);
		return &mesh;
	}
}
