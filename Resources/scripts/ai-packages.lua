function flock(fsm)
	fsm:addThread(ai.Flock.create(fsm))
end

function idle(fsm)
	fsm:addThread(ai.IdleWait.create(fsm))
end

function wander(fsm)
	fsm:addThread(ai.Wander.create(fsm))
end

function maintain_distance(fsm)
	engage = ai.MaintainDistance.makeTargetFunctionGenerator(4.5, 1.5)
	fsm:addWhileDetectHandler(GType.player, engage)
end
