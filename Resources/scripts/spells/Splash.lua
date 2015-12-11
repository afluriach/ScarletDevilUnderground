function init()
    routine = coroutine.create(Main)
end

function update()
    coroutine.resume(routine)
end

gridSize = 4
ringSize = 0.3
gridSpacing = 2
bulletSize = 0.15

function createBullet(pos)
    return createObject{name='waterBullet', type='WaterBullet', pos=pos, radius = bulletSize}
end

function Main()
    CreateGrid()
    co_wait()
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