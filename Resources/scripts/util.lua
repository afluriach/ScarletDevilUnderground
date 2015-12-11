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
