-- this function needs to be passed using registerCoroutine(<here_goes_func>)
-- to allow using: waitFor(ticks) and exitThread()
function playFolder(fileP)
	local songs = ls(fileP) --list files (ND) 
	local function basename(path)
  		return path:sub(path:find("/[^/]*$") + 1)
	end

	ND_INFO("Playing list of songs (" .. #songs .. " files)") --logging to console
    for index,value in ipairs(songs) do
        if not value.isDirectory and ends_with(value.path,".ogg") then	
            s = Music() --music handle
            s:open(value.path) --set filePath
            s:setVolume(0.5,0) --set volume=0.5 with fade_in_time = 0 seconds
            s:play(0) --start with fade_in_time = 0 seconds
			
            ND_INFO("playing " .. basename(value.path));
            while(s:isPlaying()) do
               waitFor(10) --from ND coroutine library, will wait for 10 game ticks
            end
        end
    end
end

playFolder("D:/SteamLibrary/steamapps/common/Undertale/") --quality content