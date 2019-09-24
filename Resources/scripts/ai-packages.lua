function flock(fsm)
	fsm:addThread(ai.Flock.create(fsm))
end

function idle(fsm)
	fsm:addThread(ai.IdleWait.create(fsm))
end

function wander(fsm)
	fsm:addThread(ai.Wander.create(fsm))
end
