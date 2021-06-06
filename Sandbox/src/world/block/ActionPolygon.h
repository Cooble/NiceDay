#pragma once
/*
class PolygonFactory;

struct WorldPos
{
	float x, y;
};


template<int size>
class ActionPolygon
{
private:
	WorldPos m_aray[size];
	int m_size = size;
	friend PolygonFactory;

public:
	ActionPolygon() = default;
	inline int getSize() { return m_size; }
	WorldPos& operator [](int idx) {return m_aray[idx];}

	WorldPos operator [](int idx) const {return m_aray[idx];}
};

class ActionFourgon :public ActionPolygon<4>
{

	
};

class PolygonFactory
{
private:
	static std::vector<WorldPos> s_buff;
public:
	static void start();
	static void addPoint(WorldPos pos);
	static void build(ActionFourgon&);
	
};
*/
