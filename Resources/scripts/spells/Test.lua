function init()
    log("Test init")
    log("caster is " .. GObject_getUUID(caster))
end

acc = 0

function update()
    acc = acc + 1
    if acc == 60 then
        log("update")
        acc = acc - 60
    end
end

function exit()
    log("Test end")
end