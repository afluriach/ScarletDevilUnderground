function init()
    routine = coroutine.create(Main)
end

function update()
    coroutine.resume(routine)
end

gridSize = 4
ringSize = 0.3
gridSpacing = 2
bulletSize = 0.1

function createBullet(pos)
    return createObject{name='waterBullet', type='WaterBullet', pos=pos, radius = bulletSize}
end

function Main()
    CreateGrid()
    co_wait(0.7)
    Disperse()
    co_wait(1)
    Fall()
end

function CreateGrid()
    grid = {}
    local hs = (gridSize-1)*gridSpacing / 2
    
    for y=0,gridSize-1 do
        grid[y] = {}
        for x=0,gridSize-1 do
            grid[y][x] = create_object_ring{
                center=Vector2:new(GObject_getPos(caster))+Vector2:new(x*gridSpacing-hs,y*gridSpacing-hs),
                radius=ringSize,
                count=8,
                factory=createBullet
            }
        end
    end
end

function Disperse()
    for y=0,gridSize-1 do
        for x=0,gridSize-1 do
            expand_ring(grid[y][x], 0.5)
        end
    end
end

--Apply downward force
function Fall()
    downVel = Vector2:new(0,-1.5)
    for y=0,gridSize-1 do
        for x=0,gridSize-1 do
            for idx,obj in ipairs(grid[y][x]) do
                if isValidObject(obj) then add_vel(obj, downVel) end
            end
        end
    end
end

