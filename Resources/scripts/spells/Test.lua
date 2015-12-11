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

function update()
    coroutine.resume(routine)
end

function RoutineMain()
    while true do
        InitialSpawn()
        co_wait(1)
        InitialLaunch()
        co_wait(1)
        
        local function rotateDegrees(deg)
            Rotate(math.rad(deg))
        end
        
        timed_interval(0,90,0.75,rotateDegrees)
        co_wait(0.5)
        timed_interval(90,270,1.5,rotateDegrees)
    end
end

function InitialSpawn()
    --create ring and store created objects in global
    --not the most efficient way to initialize a list-table
    objects = create_object_ring{
        center=Vector2:new(GObject_getPos(caster)),
        count=initialRingCount,
        radius=initialRingRadius,
        factory=createRingBullet
    }
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