frames_per_second = 60
seconds_per_frame = 1 / frames_per_second

function setfenv(fn, env)
  local i = 1
  while true do
    local name = debug.getupvalue(fn, i)
    if name == "_ENV" then
      debug.upvaluejoin(fn, i, (function()
        return env
      end), 1)
      break
    elseif not name then
      break
    end

    i = i + 1
  end

  return fn
end

function getfenv(fn)
  local i = 1
  while true do
    local name, val = debug.getupvalue(fn, i)
    if name == "_ENV" then
      return val
    elseif not name then
      break
    end
    i = i + 1
  end
end

function is (value, t)
	return type(value) == t
end

function isnot (value, t)
	return type(value) ~= t
end

function getobj(name, typeStr)
    --For some reason, if I pass an undefined parameter to wrapped C function,
    --it is not correctly detected as nil but instead produces a memory error.

    if isnot(name, 'string') then
        error("name must be a string")
    end

    o = getObjByName(name)

    if is(typeStr, 'string') and o then
        return convert(o,typeStr)
    else
        return o
    end
end

_coWait = 0

--Used to implement waiting in a coroutine by consuming a specific number of 
--update ticks before returning
function co_wait(seconds)
    assert_type(seconds, 'number')
    assert(seconds > 0)
    _coWait = _coWait + seconds
    while _coWait > seconds_per_frame do
        coroutine.yield()
        _coWait = _coWait - seconds_per_frame
    end
end

function exit_repl()
    exitREPL = true
end

--Repeatedly sample a function over the interval [start,end)
--The function is run once per resume
--repeatedly over time with discrete LERP values.
function timed_interval(startval, endval, lengthseconds, f)
    local range = endval - startval
    local ticksForInterval = math.floor(lengthseconds*frames_per_second)
    local dt = range / ticksForInterval
    
    local crntval = startval
    
    for count=1,ticksForInterval do
        f(crntval)
        crntval = crntval + dt
        coroutine.yield()
    end
end

function assert_type(val,type_str)
    assert(type(val) == type_str)
end

function str_cat(...)
    local result = ""
    
    for idx,val in ipairs(table.pack(...)) do
        result = result .. tostring(val)
    end
    return result
end

function assert_table_contents(t, entries)
    for idx,entry in pairs(entries) do
        if type(t[entry[1]]) ~= entry[2] then
            error(str_cat(
                "Entry ",
                entry[1],
                " of type ",
                type(t[entry[1]]),
                ", expected ",
                entry[2]
            ))
        end
    end
end

function create_object_ring(args)
--    assert_type(args, 'table')
--    assert_table_contents( args,{
--        {'count', 'number'},
--        {'radius', 'number'},
--        {'center', 'table'},
--        {'factory','function'}
--    })
    
    local objects = {}
    for i=0,args.count-1 do
        --bullet pos is really an offset from caster position
        local pos = Vector2.static.ray(args.radius, radial_angle(i/args.count))
        pos = pos + args.center        

        objects[i+1] = args.factory(pos)
    end
    return objects
end

--Apply outer radial velocity to a ring of objects.
--This assumes that the object[i]'s angular position is i/#objects*math.pi.
function expand_ring(objects, speed)
    for idx,obj in ipairs(objects) do
        local angle = radial_angle((idx-1)/ #objects)
        local vel = Vector2.static.ray(speed, angle)
        
        if isValidObject(obj) then
            GObject_setVel(obj, vel)
        end
    end
end

function add_vel(object, vel)
    local new_total = Vector2:new()
    new_total = new_total + GObject_getVel(object) + vel
    GObject_setVel(object, new_total)
end