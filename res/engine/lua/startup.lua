--runs on start of the engine
--used to load scripts

runf("res/engine/lua/vec.lua")
ND_INFO("[LUA] loaded vec lib")

function starts_with(str, start)
   return str:sub(1, #start) == start
end

function ends_with(str, ending)
   return ending == "" or str:sub(-#ending) == ending
end

corouts = {}

-- will call func in coroutine mode
-- call coroutine.yield(numberOfTicksToWait) 0 = means call next tick
-- coroutine.yield(-1) to stop immediately
function registerCoroutine(func, ticksToWait)
    ticksToWait = ticksToWait or 0
    c = {corou=coroutine.create(func),sleepy_ticks=ticksToWait}
    table.insert(corouts,c)
end

-- can be used only in coroutines
-- will wait for specified number of game ticks
--  0 -> returns immediately
function waitFor(ticks)
    if ticks>0 then 
        coroutine.yield(ticks-1)
    end
end

-- can be used only in coroutines
-- stops thread and never returns
function exitThread()
    coroutine.yield(-1)
end

function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

--printing nbt
function printNBT2(nbt,lineBlank)
	if nbt:isMap() then
		str = "{\n"
		for k, x in pairs(nbt:maps()) do
			str = str .."\t".. lineBlank .. k ..":".. printNBT2(x,lineBlank .. "\t").."\n"
		end
		str = str ..lineBlank.. '}\n'
		return str
	end
	if nbt:isArray() then
		str = "{\n"
		for i=0,#nbt do
			str = str .."\t".. lineBlank .. tostring(i) ..":".. printNBT2(nbt:nbt(i),lineBlank .. "\t").."\n"
		end
		str = str ..lineBlank.. '}\n'
		return str
	end
	if nbt:isNull() then
		return "NIL"
	end
	return tostring(nbt:getValue())
end
function printNBT(nbt)
	return "\n"..printNBT2(nbt,"")
end

function NBT:__tostring()
    return printNBT(self)
end