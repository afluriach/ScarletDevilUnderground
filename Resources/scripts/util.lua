frames_per_second = 60

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

--Used to implement waiting in a coroutine by consuming a specific number of 
--update ticks before returning
function co_wait(seconds)
    for i=1,seconds*frames_per_second do coroutine.yield() end
end

repl = coroutine.create(doREPL)

function check_repl()
    coroutine.resume(repl)
end