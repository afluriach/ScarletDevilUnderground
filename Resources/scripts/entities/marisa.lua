function init()
    print("Marisa init.")
    print(this:getUUID())
end

function onDetect(obj)
    print(obj:getName() .. " detected.")
end

function onEndDetect(obj)
    print(obj:getName() .. " lost.")
end