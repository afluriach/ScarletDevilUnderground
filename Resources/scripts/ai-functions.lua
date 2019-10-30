ai.BatEngage = class("BatEngage")

function ai.BatEngage:init(fsm, target)
	self.fsm, self.target = fsm, target
	
end

function ai.BatEngage:update()
	if self.target:isValid() then
		return push_return( ai.Flank.create(self.fsm, self.target, 2.0, 1.0) )
	else
		return pop_return()
	end
end

ai.Facer = class("Facer")

function ai.Facer:init(fsm)
	self.fsm = fsm
end

function ai.Facer:onEnter()
	self.target = self.fsm:getSpace():getPlayerAsRef()
end

function ai.Facer:update()
	if self.target:isValid() then
		local agent = self.fsm:getObject()
		local target = self.target:get()
		if ai.isFacingTarget(agent, target) then
			agent:setVel( SpaceVect.ray(agent:getMaxSpeed(), agent:getAngle() ) )
		else
			agent:setVel( SpaceVect.new() )
		end
	end
	
	return steady_return()
end

function ai.Facer:onEvent(event)
	if event:isBulletHit() then
		self.fsm:getObject():rotate( math.pi * 0.5 )
	end
	return handle
end

function ai.Facer:getEvents()
	return ai.event_type_bitfield(ai.event_type.bulletHit)
end

ai.Follower = class("Follower")

function ai.Follower:init(fsm)
	self.fsm = fsm
end

function ai.Follower:onEnter()
	self.target = self.fsm:getSpace():getPlayerAsRef()
end

function ai.Follower:update()
	if self.target:isValid() then
		local agent = self.fsm:getObject()
		local target = self.target:get()
		if ai.isFacingTargetsBack(agent, target) then
			agent:setVel( SpaceVect.ray(agent:getMaxSpeed(), agent:getAngle() ) )
		else
			agent:setVel( SpaceVect.new() )
		end
	end
	
	return steady_return()
end

function ai.Follower:onEvent(event)
	if event:isBulletHit() then
		self.fsm:getObject():rotate( math.pi * -0.5 )
	end
	return handle
end

function ai.Follower:getEvents()
	return ai.event_type_bitfield(ai.event_type.bulletHit)
end

ai.SakuyaNPC1 = class("SakuyaNPC1")

function ai.SakuyaNPC1:init(fsm)
	self.fsm = fsm
end

function ai.SakuyaNPC1:update()
	return push_return(ai.Wander.create(self.fsm, 0.25, 0.75, 4.0, 1.0))
end
