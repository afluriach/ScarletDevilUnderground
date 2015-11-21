function f()
    log "Hello, Lua"
end

function add(a,b)
    log (a+b)
    return a+b, a, b
end

f()