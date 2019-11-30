#pragma once

namespace CommonMessages {
	struct PlayMessage
	{
		enum:int
		{
			PLAY,
			CREATE,
		};
		int type;
		std::string worldName="none";
	};

}