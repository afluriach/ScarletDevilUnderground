objects.Desk = class('Desk')

function objects.Desk:init(super)
	self.super = super
end

function objects.Desk:interact(p)
	self.super.space:enterWorldSelect()
end

objects.Sign = class('Sign')

function objects.Sign:init(super)
	self.super = super
end

function objects.Sign:interact(p)
	self.super.space:createDialog('dialogs/' .. self.super:getName(), false)
end
