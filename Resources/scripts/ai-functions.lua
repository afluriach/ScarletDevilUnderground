BatEngage = class("BatEngage")

function BatEngage:init(fsm, target)
	self.fsm, self.target = fsm, target
	
end

function BatEngage:update()
	if self.target:isValid() then
		return push_return( ai.Flank.create(self.fsm, self.target, 2.0, 1.0) )
	else
		return pop_return()
	end
end

Facer = class("Facer")

function Facer:init(fsm)
	self.fsm = fsm
end

function Facer:onEnter()
	self.target = self.fsm:getSpace():getPlayerAsRef()
end

function Facer:update()
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

function Facer:onEvent(event)
	if event:isBulletHit() then
		self.fsm:getObject():rotate( math.pi * 0.5 )
	end
	return handle
end

function Facer:getEvents()
	return ai.event_type_bitfield(ai.event_type.bulletHit)
end

Follower = class("Follower")

function Follower:init(fsm)
	self.fsm = fsm
end

function Follower:onEnter()
	self.target = self.fsm:getSpace():getPlayerAsRef()
end

function Follower:update()
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

function Follower:onEvent(event)
	if event:isBulletHit() then
		self.fsm:getObject():rotate( math.pi * -0.5 )
	end
	return handle
end

function Follower:getEvents()
	return ai.event_type_bitfield(ai.event_type.bulletHit)
end
