ai.BatEngage = class("BatEngage")

function ai.BatEngage:init(super, target)
	self.super, self.target = super, target
	
end

function ai.BatEngage:update()
	if self.target:isValid() then
		return push_return( ai.Flank.create(self.super.fsm, self.target, 2.0, 1.0) )
	else
		return pop_return()
	end
end

ai.Facer = class("Facer")

function ai.Facer:init(super)
	self.super = super
end

function ai.Facer:onEnter()
	self.target = self.super:getSpace():getPlayerAsRef()
end

function ai.Facer:update()
	if self.target:isValid() then
		local agent = self.super:getObject()
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
		self.super:getObject():rotate( math.pi * 0.5 )
	end
	return handle
end

function ai.Facer:getEvents()
	return ai.event_type_bitfield(ai.event_type.bulletHit)
end

ai.Follower = class("Follower")

function ai.Follower:init(super)
	self.super = super
end

function ai.Follower:onEnter()
	self.target = self.super:getSpace():getPlayerAsRef()
end

function ai.Follower:update()
	if self.target:isValid() then
		local agent = self.super:getObject()
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
		self.super:getObject():rotate( math.pi * -0.5 )
	end
	return handle
end

function ai.Follower:getEvents()
	return ai.event_type_bitfield(ai.event_type.bulletHit)
end

ai.GreenFairy = class("GreenFairy")

function ai.GreenFairy:init(super)
	self.super = super
end

function ai.GreenFairy:onEnter()
	self.wander = ai.Wander.create(self.super.fsm, 0.75, 1.5, 2.0, 4.0)
	self.wander:onEnter()
	self.evade = ai.Evade.create(self.super.fsm, GType.playerBullet)
	self.evade:onEnter()
	self.fire = ai.FireOnStress.create(self.super.fsm, 5.0)
	self.fire:onEnter()
end

function ai.GreenFairy:update()
	self.evade:update()
	self.fire:update()

	if self.evade:isActive() then
		self.wander:reset()
	else
		self.wander:update()
	end
		
	return steady_return()
end

ai.Rumia1 = class("Rumia1")

function ai.Rumia1:init(super, target)
	self.super = super
	self.target = target
end

function ai.Rumia1:onEnter()
	self.moveFunction = ai.Flank.create(self.super.fsm, self.target, 3.0, 1.0)
	self.moveFunction:onEnter()
	self.fireFunction = ai.FireAtTarget.create(self.super.fsm, self.target)
	self.fireFunction:onEnter()
end

function ai.Rumia1:update()
	self.moveFunction = ai.autoUpdateFunction(self.moveFunction)
	self.fireFunction = ai.autoUpdateFunction(self.fireFunction)
	
	return steady_return()
end

ai.SakuyaNPC1 = class("SakuyaNPC1")

function ai.SakuyaNPC1:init(super)
	self.super = super
end

function ai.SakuyaNPC1:update()
	return push_return(ai.Wander.create(self.super.fsm, 0.25, 0.75, 4.0, 1.0))
end

ai.StalkerTeleport = class("StalkerTeleport")

function ai.StalkerTeleport:init(super)
	self.super = super
end

function ai.StalkerTeleport:onEvent(event)
	if event:getEventType() == ai.event_type.zeroStamina then
		self:applyTeleport()
		return true
	end
	return false
end

function ai.StalkerTeleport:getEvents()
	return ai.event_type_bitfield(ai.event_type.zeroStamina)
end

function ai.StalkerTeleport:applyTeleport()
	agent = self.super:getAgent()

	agent:getAttributeSystem():setFullStamina()
	agent:applyMagicEffect( app.getEffect("Teleport"), effect_attributes.new(0.0, -1.0) )
end
