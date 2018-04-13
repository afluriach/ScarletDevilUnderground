function init()
    print("Marisa init.")
    print(this:getUUID())
    
    stopDialog()
    
    speed = 3
    
    p = getPath(this:getIntPos(),{8,12})
    follow = follow_path(p,speed)
    
end

function update()
	coroutine.resume(follow)
end

function onDetect(obj)
    print(obj:getName() .. " detected on frame " .. getFrameNumber())
end

function onEndDetect(obj)
    print(obj:getName() .. " lost on frame " .. getFrameNumber())
end
