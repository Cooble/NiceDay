#pragma once
struct BlockStruct {
	int id;
	int metadata;
};
class Block
{
private:
	const int m_id;
public:
	Block(int id);
	virtual ~Block();
	inline int getID() const { return m_id; };
	virtual int getTextureOffset(int metadata) const;
};

