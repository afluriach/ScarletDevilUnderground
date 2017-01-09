--Incrementally move entity along path. Coroutine generator.
function follow_path(p, speed)
	return coroutine.create(function()
		--base case, move to first waypoint
		moveTo(this:getPos(), Vector2:new(p[1][1], p[1][2]), speed)

		for i=1, #p-1 do
			moveTo(Vector2:new(p[i][1], p[i][2]), Vector2:new(p[i+1][1], p[i+1][2]), speed)
		end
	end)
end

--Move entity by manually setting position. Coroutine helper function.
function moveTo(start, End, speed)
	--print("moveTo", start.x,start.y,End.x,End.y)
		
	dist = (End - start):length()
	time = dist / speed
	--The LERP factor is time, but it must be scaled to [0,1].
	timeInv = 1 / time

	for timeAccumulated=0,time,seconds_per_frame do
		this:setPos(Vector2.static.lerp(start, End, timeAccumulated * timeInv))
		coroutine.yield()
	end
end