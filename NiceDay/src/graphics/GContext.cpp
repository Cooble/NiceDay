#include "ndpch.h"
#include "GContext.h"
#include "platform/OpenGL/GLContext.h"

void GContext::init(GraphicsAPI api)
{
	switch (api)
	{
	case GraphicsAPI::None:
		ND_ERROR("this api not supported!");
		break;
	case GraphicsAPI::OpenGL:
		s_context = new GLContext();
		break;
	}
}
