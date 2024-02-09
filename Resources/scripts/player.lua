objects.Player = class("Player")

function objects.Player:init(super)
	self.super = super
end

function objects.Player:initialize_ai()
	self.super:setAIFunction( ai.PlayerControl.create(self.super.fsm) )
	app.log("player.initialize_ai")
end