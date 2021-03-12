ai.fsm = class('fsm')

function ai.fsm:init(super)
	self.super = super
end

function ai.fsm:detectEnemy(enemy)
	if self.detectEnemyHandler then
		self.super:pushFunction( self.detectEnemyHandler(self.super, enemy) )
	end
end

ai.ghost_fairy = ai.fsm:extend('ghost_fairy')

function ai.ghost_fairy:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("GhostFairyEngage")
end
