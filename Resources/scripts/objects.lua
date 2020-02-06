objects.Desk = class('Desk')

function objects.Desk:init(super)
	self.super = super
end

function objects.Desk:interact(p)
	self.super.space:enterWorldSelect()
end
