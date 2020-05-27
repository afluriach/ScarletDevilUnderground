spells.BlueFairyBomb = class('BlueFairyBomb', {
	name = 'BlueFairyBomb',
	description = '',
	damage = DamageInfo.new(10.0, DamageType.effectArea),
	radius = 2.5,
	length = 0.75,
	angularSpeed = math.pi * 0.5,
	init = function(self, super)
		self.super = super
	end,
	onEnter = function(self)
		self.sprite = self.super.space:createSprite(
			'blue_explosion',
			GraphicsLayer.agentOverlay,
			self.super.object:getPos(),
			self.radius * 0.5
		)
		util.radialDamageArea(self.super.object, self.radius, GType.player, self.damage)
	end,
	update = function(self)
		self.super.space:setRotation(self.sprite, self.angularSpeed * self.super.time)
	end,
	onExit = function(self)
		self.super.space:removeSprite(self.sprite)
	end
})
