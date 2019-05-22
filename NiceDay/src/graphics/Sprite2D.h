#pragma once
#include "ndpch.h"//todo add glm to ndpch

#include "graphics/Program.h"
#include "graphics/buffers/VertexArray.h"
#include "graphics/buffers/VertexBuffer.h"

class Sprite2D
{
private:
  static VertexBuffer* s_vbo;
  static VertexArray* s_vao;
  static Program* s_program;
  
private:
  Texture m_texture;
  glm::vec2 m_scale;
  glm::vec2 m_position;
  glm::mat4 m_model_matrix;
  bool m_stale_matrix;
public:
  static void init()
  {
      float quad[] = {
      1,0,
      1,1,
      0,0,
      0,1,
    };
    s_vbo = new VertexBuffer(quad, sizeof(quad));
    VertexBufferLayout l;
    l.push<float>(2);
    s_vao = new VertexArray();
    s_vao->addBuffer(*s_vbo, l);
    
    s_program = new Program("res/shaders/Sprite2D.shader");
    s_program->bind();
	  s_program->setUniform1i("u_texture", 0);
	  s_program->unbind();
 
  }
  private:
    void computeMatrix();
  public:
    Sprite2D(Texture*);
    
    void setPosition(glm::vec2);
    void setScale(glm::vec2);
   
    
    
    
    
    
  
  





}
