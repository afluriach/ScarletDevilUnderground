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

--Wrap C API so that it uses Vector2.
GObject._getPos = GObject.getPos
GObject._setPos = GObject.setPos

GObject.getPos = function(self)
	v = self:_getPos()
	print('getPos', v.x, v.y)
	setmetatable(v, Vector2)
	return v
end

GObject.getIntPos = function(self)
--	return self:getPos():getFloor()
	v = self:_getPos()
	return Vector2:new(math.floor(v.x), math.floor(v.y))
end

GObject.setPos = function(self, VectOrX, y)
	if type(VectOrX) == "number" then
		self:_setPos(VectOrX, y)
	else
		self:_setPos(VectOrX.x, VectOrX. y)
	end
end