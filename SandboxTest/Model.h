#pragma once

class Mesh;
class Material;
class Model
{
private:
	Mesh* m_mesh;
	Material* m_material;
public:
	Model(Mesh* mesh,Material* material):m_mesh(mesh),m_material(material){}

	const Mesh* mesh() const { return m_mesh; }
	const Material* material() const { return m_material; }
};
