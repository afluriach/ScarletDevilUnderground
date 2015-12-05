initialRingRadius = 1.5
initialRingCount = 16
initialLaunchSpeed = 3

function init()
    log("Test init")
    log("caster is " .. GObject_getUUID(caster))
    
    routine = coroutine.create(RoutineMain)
end

function createRingBullet(pos)
    log("createRingBullet")
    return createObject{name='b', type='FireBullet', pos=pos, radius = 0.15}
end

function update()
    coroutine.resume(routine)
end

function RoutineMain()
    while true do
        InitialSpawn()
        co_wait(2)
        InitialLaunch()
        co_wait(3)
    end
end

function InitialSpawn()
    --create ring and store created objects in global
    --not the most efficient way to initialize a list-table    
    objects = {}
    --Warning, C API does not return a Lua Vector2 but a duck-typed table.
    caster_pos = Vector2:new(GObject_getPos(caster))
    log("caster pos: " .. caster_pos.x .. "," .. caster_pos.y)
    
    for i=0,initialRingCount-1 do
        --bullet pos is really an offset from caster position
        pos = Vector2.static.ray(initialRingRadius, i/initialRingCount*math.pi*2)
        log("bullet " .. i .. " offset " .. pos.x .. "," .. pos.y)
        pos = pos + caster_pos        

        objects[i+1] = createRingBullet(pos)
    end    
end

function InitialLaunch()
    --launch ring
    
    log("initial launch")
    
    for idx=0,initialRingCount-1 do
        --their direction relative to the caster is also the 
        --direction they will be launched
    
        --since this is before rotation, it can just be determined programmatically
        
        angle = (idx) / initialRingCount * math.pi*2
        vel = Vector2.static.ray(initialLaunchSpeed, angle)
        
        log(idx+1 .. " vel set " .. vel.x .. "," .. vel.y)
    
        GObject_setVel(objects[idx+1], vel)
    end
end

function exit()
    log("Test end")
end