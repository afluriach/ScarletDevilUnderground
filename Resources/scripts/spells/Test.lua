initialRingRadius = 0.7
initialRingCount = 12
initialLaunchSpeed = 3

function init()
    log("Test init")
    log("caster is " .. GObject_getUUID(caster))
    
    routine = coroutine.create(RoutineMain)
end

function createRingBullet(pos)
    return createObject{name='fireBullet', type='FireBullet', pos=pos, radius = 0.1}
end

function createRing(caster_pos)
    local objects = {}
    for i=0,initialRingCount-1 do
        --bullet pos is really an offset from caster position
        local pos = Vector2.static.ray(initialRingRadius, radial_angle(i/initialRingCount))
        pos = pos + caster_pos        

        objects[i+1] = createRingBullet(pos)
    end
    return objects
end

function update()
    coroutine.resume(routine)
end

function RoutineMain()
    while true do
        InitialSpawn()
        co_wait(1)
        InitialLaunch()
        co_wait(1)
        
        for i=0,90,6 do
            Rotate(math.rad(i))
            co_wait(0.05)
        end
        co_wait(0.5)
        for i=90,270,6 do
            Rotate(math.rad(i))
            co_wait(0.05)
        end
    end
end

function InitialSpawn()
    --create ring and store created objects in global
    --not the most efficient way to initialize a list-table
    objects = createRing(Vector2:new(GObject_getPos(caster)))
end

function InitialLaunch()
    --launch ring
    for idx=0,initialRingCount-1 do
        --their direction relative to the caster is also the 
        --direction they will be launched
    
        --since this is before rotation, it can just be determined programmatically        
        local angle = radial_angle(idx/ initialRingCount)
        local vel = Vector2.static.ray(initialLaunchSpeed, angle)
        
        if isValidObject(objects[idx+1]) then
            GObject_setVel(objects[idx+1], vel)
        end
    end
end

function Rotate(rotation)
    for idx=0,initialRingCount-1 do
        --since this is before rotation, it can just be determined programmatically
        
        --the direction they are already moving in
        local angle = radial_angle(idx/initialRingCount)
        --add rotation
        angle = angle + rotation
        local vel = Vector2.static.ray(initialLaunchSpeed, angle)
        
        if isValidObject(objects[idx+1]) then
            GObject_setVel(objects[idx+1], vel)
        end
    end

end

function exit()
    log("Test end")
end