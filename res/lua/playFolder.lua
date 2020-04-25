
function playFolder(fileP)

	local songs = ls(fileP)
	local function basename(path)
  		return path:sub(path:find("/[^/]*$") + 1)
	end

	ND_INFO("Playing list of songs (" .. #songs .. " files)")
	musicIndex=0
    total=0;
    maxi =100;
    min=0;
    offset = 0;
    for index,value in ipairs(songs) do
        if min<offset then
            min= min+1;
            goto continue;
        end
        if not value.isDirectory and ends_with(value.path,".ogg") then	
            if total<maxi then
				musicIndex = musicIndex + 1
                s = Music()
                s:open(value.path)
                s:setVolume(0.5,0)             
s:play(0)
				
                ND_INFO("playing (" .. tostring(musicIndex) ..") ".. basename(value.path));
                while(s:isPlaying()) do
                   waitFor(10)
                end
            end
        total= total+1;		
        end
     ::continue::
    end
end

playFolder("D:/SteamLibrary/steamapps/common/Undertale/")

