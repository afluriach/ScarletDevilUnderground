effects.FreezeStatus = class('FreezeStatus')

function effects.FreezeStatus.getFlags()
	return MagicEffect.make_flags_bitfield(MagicEffect_flags.timed)
end

function effects.FreezeStatus:init(super, target)
    self.target = target:getAsAgent()
end

function effects.FreezeStatus:onEnter()
    self.target:addGraphicsAction(graphics.freezeEffectAction(), 0)
    self.target:setFrozen(true)
    self.target:setFiringSuppressed(true)
    self.target:setMovementSuppressed(true)
end

function effects.FreezeStatus:onExit()
    self.target:addGraphicsAction(graphics.freezeEffectEndAction(), 0)
    self.target:setFrozen(false)
    self.target:setFiringSuppressed(false)
    self.target:setMovementSuppressed(false)
end

effects.DarknessCurse = class('DarknessCurse')

function effects.DarknessCurse.getFlags()
	return MagicEffect.make_flags_bitfield(
		MagicEffect_flags.indefinite,
		MagicEffect_flags.active
	)
end

function effects.DarknessCurse:init(super, target)
	self.super = super
	self.agent = target:getAsAgent()
end

function effects.DarknessCurse:onEnter(target)
	self.agent:setInhibitSpellcasting(true)
	self.agent:stopSpell()
	self.agent:addGraphicsAction(graphics.darknessCurseFlickerTintAction(), 0)
end

function effects.DarknessCurse:update()
	as = self.agent:getAttributeSystem()
	
	if as:get(Attribute.darknessDamage) <= 0.0 then
		self.super:remove()
	end
	as:timerDecrement(Attribute.darknessDamage, 9.0)
end

function effects.DarknessCurse:onExit()
	self.agent:setInhibitSpellcasting(false)
	self.agent:stopGraphicsAction(cocos_action_tag.darkness_curse, 0)
end

effects.GhostProtection = class('GhostProtection')

function effects.GhostProtection.getFlags()
	return MagicEffect.make_flags_bitfield(
		MagicEffect_flags.indefinite,
		MagicEffect_flags.active
	)
end

function effects.GhostProtection:init(super, target)
	self.super = super
	self.agent = target:getAsAgent()
end

function effects.GhostProtection:onEnter()
	self.crntHP = self.agent:getAttribute(Attribute.hp)
	self.accumulator = 0
end

function effects.GhostProtection:update()
	_hp = self.agent:getAttribute(Attribute.hp)
	delta = self.crntHP - _hp
	self.accumulator = self.accumulator + delta / self.agent:getAttribute(Attribute.maxHP) * 12.5
	
	if self.accumulator >= 1.0 and not self.agent:getAttributeSystem():hasHitProtection() then
		self.agent:addGraphicsAction(graphics.flickerAction(0.25, 5.0, 128), 0 )
		self.agent:getAttributeSystem():setTimedProtection(5.0)
		self.accumulator = 0
	end
	
	self.crntHP = _hp
	self.accumulator = timerDecrement(self.accumulator, 0.5)
end

effects.RedFairyStress = class('RedFairyStress')

function effects.RedFairyStress.getFlags()
	return MagicEffect.make_flags_bitfield(
		MagicEffect_flags.indefinite,
		MagicEffect_flags.active
	)	
end

function effects.RedFairyStress:init(super, target)
	self.agent = target:getAsAgent()
end

function effects.RedFairyStress:onEnter()
	self.baseAttackSpeed = self.agent:getAttribute(Attribute.attackSpeed)
end

function effects.RedFairyStress:update()
	as = self.agent:getAttributeSystem()
end

function effects.RedFairyStress:onExit()
	self.agent:getAttributeSystem():set(Attribute.attackSpeed, self.baseAttackSpeed)
end
