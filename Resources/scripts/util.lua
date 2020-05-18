function timerDecrement(x, scale)
	local _scale = scale
	if not _scale then
		local _scale = 1.0
	end
	
	return math.max(x - App.getParams().secondsPerFrame*scale, 0.0)
end

function timerIncrement(x, scale)
	local _scale = scale
	if not _scale then
		local _scale = 1.0
	end
	
	return x + App.getParams().secondsPerFrame*scale
end

function intervalMapIndex(map, val)
	for i,v in ipairs(map) do
		if val >= v[1] and val < v[2] then
			return v[3]
		end
	end

	return nil
end
