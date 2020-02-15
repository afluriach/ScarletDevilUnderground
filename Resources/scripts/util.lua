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
