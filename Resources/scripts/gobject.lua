--This wraps the call to _callScriptVal to perform serialization wrap.
function GObject.callScriptVal(self, name, ...)
    args_str = serpent.dump(table.pack(...))
    local result_str = self:_callScriptVal(name,args_str)
    local success,result = serpent.load(result_str)
    if success then
        return table.unpack(result)
    else
        print("GObject.callScriptVal: error deserializing result string " + result_str) 
    end
end