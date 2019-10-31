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
