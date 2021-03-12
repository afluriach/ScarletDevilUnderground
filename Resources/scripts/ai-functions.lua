ai.BatEngage = class("BatEngage")

function ai.BatEngage:init(super, target)
	self.super, self.target = super, target
	
	self.moveFunction = ai.Flank.create(self.super.fsm, self.target, 2.0, 1.0)
end

function ai.BatEngage:onEnter()
	self.moveFunction:onEnter()
end

function ai.BatEngage:update()
	if not self.target:isValid() then
		return ai.update_return.makePop()
	end
	
	self.moveFunction:update()
	return ai.update_return.makeSteady(0.0)
end

ai.Facer = class("Facer")

function ai.Facer:init(super)
	self.super = super
end

function ai.Facer:update()
	if not self.target or not self.target:isValid() then
		self.target = self.super.space:getPlayerAsRef()
	end

	if self.target and self.target:isValid() then
		local agent = self.super.object
		local target = self.target:get()
		if ai.isFacingTarget(agent, target) then
			agent:setVel( SpaceVect.ray(agent:getMaxSpeed(), agent:getAngle() ) )
		else
			agent:setVel( SpaceVect.new() )
		end
	end
	
	return ai.update_return.makeSteady(0.0)
end

function ai.Facer:bulletHit(b)
	self.super.object:rotate( math.pi * 0.5 )
end

ai.Follower = class("Follower")

function ai.Follower:init(super)
	self.super = super
end

function ai.Follower:update()
	if not self.target or not self.target:isValid() then
		self.target = self.super.space:getPlayerAsRef()
	end
		
	if self.target and self.target:isValid() then
		local agent = self.super.object
		local target = self.target:get()
		if ai.isFacingTargetsBack(agent, target) then
			agent:setVel( SpaceVect.ray(agent:getMaxSpeed(), agent:getAngle() ) )
		else
			agent:setVel( SpaceVect.new() )
		end
	end
	
	return ai.update_return.makeSteady(0.0)
end

function ai.Follower:bulletHit(b)
	self.super.object:rotate( math.pi * -0.5 )
end

ai.FairyEngage = class("FairyEngage")

function ai.FairyEngage:init(super, target)
	self.super = super
	self.target = target
end

function ai.FairyEngage:onEnter(super, target)
	self.moveFunction = ai.MaintainDistance.create(self.super.fsm, self.target, 3.0, 1.0)
	self.moveFunction:onEnter()
end

function ai.FairyEngage:update()
	self.moveFunction:update()
	self.super:aimAtTarget(self.target)
	self.super:fire()
	
	return ai.update_return.makeSteady(0.0)
end

ai.GhostFairyEngage = class("GhostFairyEngage")

function ai.GhostFairyEngage:init(super, target)
	self.super = super
	self.target = target
end

function ai.GhostFairyEngage:onEnter(super, target)
	self.moveFunction = ai.Flank.create(self.super.fsm, self.target, 4.0, 0.75)
	self.moveFunction:onEnter()
end

function ai.GhostFairyEngage:update()
	self.moveFunction:update()
	self.super:aimAtTarget(self.target)
	self.super:fire()
	
	return ai.update_return.makeSteady(0.0)
end

ai.GreenFairy = class("GreenFairy")

function ai.GreenFairy:init(super)
	self.super = super
	self.bulletDetect = false
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

	local evadeResult = self.evade:update()
	
	if evadeResult:isSteady() then
		self.wander:reset()
		return ai.update_return.makePush(self.evade)
	else
		self.fire:update()
		self.wander:update()
			
		return ai.update_return.makeSteady(0.0)
	end
end

ai.RedFairyEngage = class("RedFairyEngage")

function ai.RedFairyEngage:init(super, target)
	self.super = super
	self.target = target
end

function ai.RedFairyEngage:onEnter()
	self.bombFunction = ai.ThrowBombs.create(
		self.super.fsm,
		self.target,
		app.getBomb("RedFairyBomb"),
		4.0,
		4.0
	)
	self.bombFunction:onEnter()
	
	self.moveFunction = ai.MaintainDistance.create(self.super.fsm, self.target, 3.0, 0.5)
	self.moveFunction:onEnter()
end

function ai.RedFairyEngage:update()
	self.bombFunction:update()
	self.moveFunction:update()
	
	self.super:aimAtTarget(self.target)
	self.super:fire()
	
	return ai.update_return.makeSteady(0.0)
end

ai.BlueFairy = class("BlueFairy")

function ai.BlueFairy:init(super)
	self.super = super
end

function ai.BlueFairy:onEnter()
	local path = self.super.space:getPath(self.super.object.name)
	if path then
		self.pathFunction = ai.FollowPath.create(self.super.fsm, path, true, true)
		self.pathFunction:onEnter()
	end
	
	self.lookFunction = ai.LookTowardsFire.create(self.super.fsm, true)
	self.lookFunction:onEnter()
	self.fireFunction = ai.FireOnStress.create(self.super.fsm, 5.0)
	self.fireFunction:onEnter()
	self.powerAttackFunction = ai.ScriptFunction.create(self.super.fsm, "BlueFairyBomb")
	self.powerAttackFunction:onEnter()
end

function ai.BlueFairy:update()
	
	if self.pathFunction then
		self.pathFunction:update()
	end
	
	self.lookFunction:update()
	self.fireFunction:update()
	self.powerAttackFunction:update()
	
	return ai.update_return.makeSteady(0.0)
end

--When target (player) gets and stays within range for a certain time, it activates.
ai.BlueFairyBomb = class("BlueFairyBomb")

ai.BlueFairyBomb.cooldown = 4.0
ai.BlueFairyBomb.triggerLength = 1.0
ai.BlueFairyBomb.triggerDist = 2.0
ai.BlueFairyBomb.cost = 15.0

function ai.BlueFairyBomb:init(super)
	self.super = super
	self.accumulator = 0.0
	self.timer = self.cooldown
	self.spellID = 0
end

function ai.BlueFairyBomb:update()
	local targetDist = self.super.agent:getSensedObjectDistance(GType.player)
	
	if not self.super:isSpellActive() then
		self.timer = util.timerDecrement(self.timer)
	end
	
	if not util.isinf(targetDist) and targetDist < self.triggerDist then
		self.accumulator = util.timerIncrement(self.accumulator)
	else
		self.accumulator = 0.0
	end
	
	if
		not self.super:isSpellActive() and
		self.super.agent:get(Attribute.mp) > self.cost and
		self.accumulator >= self.triggerLength and
		self.timer <= 0.0
	then
		self:activate()
	end
	
	return ai.update_return.makeSteady(0.0)
end

function ai.BlueFairyBomb:activate()
	if self.super:castSpell( app.getSpell('BlueFairyBomb') ) then
		self.accumulator = 0.0
		self.timer = self.cooldown
	end
end

ai.ZombieFairy = class("ZombieFairy")

function ai.ZombieFairy:init(super, target)
	self.super, self.target = super, target
	
	self.moveFunction = ai.Seek.create(self.super.fsm, self.target:get(), true, 0.0)
end

function ai.ZombieFairy:onEnter()
	self.moveFunction:onEnter()	
end

function ai.ZombieFairy:update()
	self.moveFunction:update()
	return ai.update_return.makeSteady(0.0)
end

ai.MarisaForestMain = class("MarisaForestMain")

function ai.MarisaForestMain:init(super, target)
	self.super = super
	self.target = target
end

function ai.MarisaForestMain:update()	
	if not self.target or not self.target:isValid() then
		return ai.update_return.makePop()
	end
	
	self.super:aimAtTarget(self.target)
	
	if not self.super:isSpellActive() then
		self.super:castSpell(app.getSpell('StarlightTyphoon'))
	end

	return ai.update_return.makeSteady(0.0)
end

ai.PatchouliEnemy = class("PatchouliEnemy")

ai.PatchouliEnemy.intervals = {
	{200.0, 250.0, "FireStarburst"},
	{150.0, 180.0, "FlameFence"},
	{100.0, 150.0, "Whirlpool1"},
	{0.0,   50.0,  "Whirlpool2"}
}

function ai.PatchouliEnemy:init(super, target)
	self.super = super
		
	self.castFunction = ai.HPCastSequence(self.super, self.intervals)
end

function ai.PatchouliEnemy:update()
	self.castFunction:update()

	return ai.update_return.makeSteady(0.0)
end

ai.ReimuEnemy = class("ReimuEnemy")

function ai.ReimuEnemy:init(super, target)
	self.super = super
	self.target = target
	self.moveFunction = ai.Flank.create(self.super.fsm, self.target, 3.0, 2.0)
end

function ai.ReimuEnemy:onEnter()
	self.super:castSpellManual(app.getSpell("YinYangOrbs"))
	self.moveFunction:onEnter()
end

function ai.ReimuEnemy:update()
	self.moveFunction:update()
	self.super:aimAtTarget(self.target)
	self.super:fire()
	
	return ai.update_return.makeSteady(0.0)
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
	
	return ai.update_return.makeSteady(0.0)
end

ai.SakuyaMain = class("SakuyaMain")

function ai.SakuyaMain:init(super, target)
    self.super = super
	self.target = target
end

function ai.SakuyaMain:update()
	if not self.super:isSpellActive() then
		self.super:castSpell(app.getSpell('IllusionDial'))
	end
	return ai.update_return.makeSteady(0.0)
end

ai.SakuyaNPC1 = class("SakuyaNPC1")

function ai.SakuyaNPC1:init(super)
	self.super = super
	self.moveFunction = ai.Wander.create(self.super.fsm, 0.25, 0.75, 4.0, 1.0)
end

function ai.SakuyaNPC1:onEnter()
	self.moveFunction:onEnter()
end

function ai.SakuyaNPC1:update()
	self.moveFunction:update()
	return ai.update_return.makeSteady(0.0)
end

ai.StalkerTeleport = class("StalkerTeleport")

function ai.StalkerTeleport:init(super)
	self.super = super
end

function ai.StalkerTeleport:zeroStamina()
	self:applyTeleport()
end

function ai.StalkerTeleport:getTeleportPosition()
	return self.super.space:getRandomWaypoint('TeleportPad')
end

function ai.StalkerTeleport:applyTeleport()
	agent = self.super.agent

	agent:getAttributeSystem():setFullStamina()
	agent:teleport( self:getTeleportPosition() )
end

ai.HPCastSequence = class("HPCastSequence")

function ai.HPCastSequence:init(super, intervalSpellMap)
	self.super, self.intervalSpellMap = super, intervalSpellMap
end

function ai.HPCastSequence:getSpellIndex()
	local hp = self.super.agent:get(Attribute.hp)
	return intervalMapIndex(self.intervalSpellMap, hp)
end

function ai.HPCastSequence:update()
	local newSpell = self:getSpellIndex()
	
	if newSpell ~= self.crntSpell and not self.crntSpell then
		self.super:stopSpell()
	end
	
	if newSpell ~= self.crntSpell and newSpell then
		self.super:castSpell( app.getSpell(newSpell) )
	end
	
	self.crntSpell = newSpell
	return ai.update_return.makeSteady(0.0)
end

function ai.HPCastSequence:onExit()
	if self.crntSpell then
		self.super:stopSpell()
	end
end
