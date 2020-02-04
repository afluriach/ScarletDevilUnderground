objects.Spellcard = class('Spellcard')

function objects.Spellcard:init(super)
	self.super = super
end

function objects.Spellcard:initialize()
	self.super:addGraphicsAction(graphics.spellcardFlickerTintAction())
end

function objects.Spellcard:onAcquire(p)
	p:equipItems()
end
