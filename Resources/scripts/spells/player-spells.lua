spells.DarkMist = class('DarkMist', {
	name = 'Dark Mist',
	description = 'Become inivisble.',
	getParams = function()
		return spell_params.new(-1.0, -1.0, spell_cost.ongoingMP(7.5))
	end,
	init = function(self, super)
		self.super = super
	end,
	onEnter = function(self)
		caster = self.super.object
		caster:setSpriteOpacity(128)
		caster:getAsAgent():increment(Attribute.invisibility)
	end,
	onExit = function(self)
		caster = self.super.object
		caster:setSpriteOpacity(255)
		caster:getAsAgent():decrement(Attribute.invisibility)
	end
})
