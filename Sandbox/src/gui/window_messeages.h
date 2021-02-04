#pragma once
#include "memory/stack_allocator.h"

enum WindowMess :int {
	OpensFirst,
	OpenMain,
	OpenWorldSelection,
	OpenPlayWorld,
	OpenExit,
	OpenSettings,
	OpenControls,
	OpenLanguage,
	OpenBack,
	OpenBackToMain,
  OpenPause,
  OpenSkin,
	OpensLast,


	ActionsFirst,
	ActionGenerateWorld,
	ActionDeleteWorld,
	ActionWorldQuit,
	ActionsLasts
};

namespace WindowMessageData
{
	struct World
	{
		nd::temp_string worldName;
	};
	inline bool isOpenMessage(WindowMess mess) { return mess > OpensFirst && mess < OpensLast; }
	inline bool isActionMessage(WindowMess mess) { return mess > ActionsFirst && mess < ActionsLasts; }
};
