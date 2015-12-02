function getobj(name, typeStr)
    --For some reason, if I pass an undefined parameter to wrapped C function,
    --it is not correctly detected as nil but instead produces a memory error.

    if type(name) ~= 'string' then
        error("name must be a string")
    end

    o = getObjByName(name)

    if type(typeStr) == 'string' and o then
        return convert(o,type)
    else
        return o
    end
end