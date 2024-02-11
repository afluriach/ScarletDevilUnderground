objects.Player = class("Player")

function objects.Player:init(super)
	self.super = super
end

function objects.Player:initialize()
	self.controlFunction = ai.PLayerControl.create(self.super:getAsObject())
end

function objects.Player:update()
	self.controlFunction:update()
end