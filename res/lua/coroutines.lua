
--looks through all registered coroutes, resumes them and kill them if neccessary


local n = #corouts
for i=1,n do
    if corouts[i] ~= nil then
        local sleep = corouts[i]["sleepy_ticks"]
        if sleep>0 then
            corouts[i]["sleepy_ticks"] = sleep - 1;
        else
            local _,timee = coroutine.resume(corouts[i]["corou"])
            corouts[i]["sleepy_ticks"] = timee
            if coroutine.status(corouts[i]["corou"])=="dead" or corouts[i]["sleepy_ticks"]==-1 then
               corouts[i]=nil
            end
        end
    end
end

--clear the nil values
local j=0
for i=1,n do
        if corouts[i]~=nil then
                j=j+1
                corouts[j]=corouts[i]
        end
end
for i=j+1,n do
        corouts[i]=nil
end
