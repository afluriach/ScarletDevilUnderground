effects.BatTransform = class('BatTransform', {
	flags = effect_flags.indefinite,
	init = function(self, super)
		self.super = super
	end,
	onEnter = function(self)
		local a = self.super.target:getAsAgent()
		
		a:setSprite(app.getSprite('flandre_bat'))
		a:setSpriteZoom(4.0)

		a:increment(Attribute.hitProtection)		
		a:increment(Attribute.inhibitFiring)
		a:modifyAttribute(Attribute.agility, 1.5)
		a:setLayers(PhysicsLayers.ground)
	end,
	onExit = function(self)
		local a = self.super.target:getAsAgent()
		
		a:setSprite(app.getSprite('flandre'))
		a:setSpriteZoom(1.0)

		a:decrement(Attribute.hitProtection)				
		a:decrement(Attribute.inhibitFiring)
		a:modifyAttribute(Attribute.agility, -1.5)
		a:setLayers( PhysicsLayers.floor | PhysicsLayers.ground )
	end
})

effects.FreezeStatus = class('FreezeStatus', {
	flags = effect_flags.timed,
	init = function(self, super)
		self.super = super
		self.agent = self.super.target:getAsAgent()
	end,
	onEnter = function(self)
		self.agent:addGraphicsAction(graphics.freezeEffectAction())
		self.agent:setFrozen(true)
		self.agent:increment(Attribute.inhibitFiring)
		self.agent:increment(Attribute.inhibitMovement)
	end,
	onExit = function(self)
		self.agent:addGraphicsAction(graphics.freezeEffectEndAction())
		self.agent:setFrozen(false)
		self.agent:decrement(Attribute.inhibitFiring)
		self.agent:decrement(Attribute.inhibitMovement)
	end
})

effects.DarknessCurse = class('DarknessCurse', {
	flags = effect_flags.indefinite | effect_flags.active,
	init = function(self, super)
		self.super = super
		self.agent = self.super.target:getAsAgent()
	end,
	onEnter = function(self, target)
		self.agent:increment(Attribute.inhibitSpellcasting)
		self.agent:addGraphicsAction(graphics.darknessCurseFlickerTintAction())
	end,
	update = function(self)
		as = self.agent:getAttributeSystem()
		
		if as:get(Attribute.darknessDamage) <= 0.0 then
			self.super:remove()
		end
		as:timerDecrement(Attribute.darknessDamage, 9.0)
	end,
	onExit = function(self)
		self.agent:decrement(Attribute.inhibitSpellcasting)
		self.agent:stopGraphicsAction(cocos_action_tag.darkness_curse)
	end
})

effects.GhostProtection = class('GhostProtection', {
	flags = effect_flags.indefinite | effect_flags.active,
	init = function(self, super)
		self.super = super
		self.agent = super.target:getAsAgent()
	end,
	onEnter = function(self)
		self.crntHP = self.agent:get(Attribute.hp)
		self.accumulator = 0
	end,
	update = function(self)
		_hp = self.agent:get(Attribute.hp)
		delta = self.crntHP - _hp
		self.accumulator = self.accumulator + delta / self.agent:get(Attribute.maxHP) * 12.5
		
		if self.accumulator >= 1.0 and self.agent:getAttributeSystem():isZero(Attribute.hitProtection) then
			self.agent:addGraphicsAction(graphics.flickerAction(0.25, 5.0, 128) )
			self.agent:applyMagicEffect(
				app.getEffect("SetHitProtection"),
				effect_attributes.new(0.0, 5.0)
			)
			self.accumulator = 0
		end
		
		self.crntHP = _hp
		self.accumulator = timerDecrement(self.accumulator, 0.5)
	end
})

