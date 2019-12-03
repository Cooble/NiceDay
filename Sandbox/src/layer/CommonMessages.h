#pragma once


namespace CommonMessages {
	
	struct WorldMessage
	{
		inline static const char* NAME = "WorldMessage";
		
		enum :int {
			LoadWorld,
			CreateWorld,
			DestroyWorld
		};
		int type;
		std::string_view worldName;
	};

}