effects.FreezeStatus = class('FreezeStatus', {
	flags = effect_flags.timed,
	init = function(self, super)
		self.super = super
		self.agent = self.super.target:getAsAgent()
	end,
	onEnter = function(self)
		self.agent:addGraphicsAction(graphics.freezeEffectAction(), 0)
		self.agent:setFrozen(true)
		self.agent:setFiringSuppressed(true)
		self.agent:setMovementSuppressed(true)
	end,
	onExit = function(self)
		self.agent:addGraphicsAction(graphics.freezeEffectEndAction(), 0)
		self.agent:setFrozen(false)
		self.agent:setFiringSuppressed(false)
		self.agent:setMovementSuppressed(false)
	end
})

effects.DarknessCurse = class('DarknessCurse', {
	flags = effect_flags.indefinite | effect_flags.active,
	init = function(self, super)
		self.super = super
		self.agent = target:getAsAgent()
	end,
	onEnter = function(self, target)
		self.agent:setInhibitSpellcasting(true)
		self.agent:stopSpell()
		self.agent:addGraphicsAction(graphics.darknessCurseFlickerTintAction(), 0)
	end,
	update = function(self)
		as = self.agent:getAttributeSystem()
		
		if as:get(Attribute.darknessDamage) <= 0.0 then
			self.super:remove()
		end
		as:timerDecrement(Attribute.darknessDamage, 9.0)
	end,
	onExit = function(self)
		self.agent:setInhibitSpellcasting(false)
		self.agent:stopGraphicsAction(cocos_action_tag.darkness_curse, 0)
	end
})

effects.GhostProtection = class('GhostProtection', {
	flags = effect_flags.indefinite | effect_flags.active,
	init = function(self, super)
		self.super = super
		self.agent = super.target:getAsAgent()
	end,
	onEnter = function(self)
		self.crntHP = self.agent:getAttribute(Attribute.hp)
		self.accumulator = 0
	end,
	update = function(self)
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
})

