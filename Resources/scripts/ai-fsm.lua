ai.fsm = class('fsm')

function ai.fsm:init(super)
	self.super = super
end

function ai.fsm:detectEnemy(enemy)
	if self.engageDialog then
		self.super.space:createDialog( self.engageDialog, false )
	end

	if self.detectEnemyHandler then
		self.super:pushFunction( self.detectEnemyHandler(self.super, enemy) )
	end
end

function ai.fsm:enemyRoomAlert(enemy)
	if self.enemyRoomAlertHandler then
		self.super:pushFunction( self.enemyRoomAlertHandler(self.super, enemy) )
	end
end

function ai.fsm:zeroHP()
	if self.defeatDialog then
		self.super.space:createDialog( self.defeatDialog, false )
	end
end

ai.bat = ai.fsm:extend('bat')

function ai.bat:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("BatEngage")
end

ai.facer = ai.fsm:extend('facer')

function ai.facer:initialize()
	self.super:pushFunction( ai.ScriptFunction.create(self.super, "Facer") )
end

ai.follower = ai.fsm:extend('follower')

function ai.follower:initialize()
	self.super:pushFunction( ai.ScriptFunction.create(self.super, "Follower") )
end

ai.ghost_fairy = ai.fsm:extend('ghost_fairy')

function ai.ghost_fairy:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("GhostFairyEngage")
end

ai.ice_fairy = ai.fsm:extend('ice_fairy')

function ai.ice_fairy:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("FairyEngage")
end

ai.zombie_fairy = ai.fsm:extend('zombie_fairy')

function ai.zombie_fairy:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("ZombieFairy")
end

ai.forest_marisa = ai.fsm:extend('forest_marisa')

function ai.forest_marisa:initialize()
	self.enemyRoomAlertHandler = ai.ScriptFunction.targetGenerator("MarisaForestMain")
end

ai.patchouli_enemy = ai.fsm:extend('patchouli_enemy')

function ai.patchouli_enemy:initialize()
	self.enemyRoomAlertHandler = ai.ScriptFunction.targetGenerator("PatchouliEnemy")
end

ai.reimu_enemy = ai.fsm:extend('reimu_enemy')

ai.reimu_enemy.engageDialog = 'dialogs/reimu_forest_pre_fight'
ai.reimu_enemy.defeatDialog = 'dialogs/reimu_forest_post_fight'

function ai.reimu_enemy:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("ReimuEnemy")
end

ai.sakuya = ai.fsm:extend('sakuya')

function ai.sakuya:initialize()
	self.enemyRoomAlertHandler = ai.ScriptFunction.targetGenerator("SakuyaMain")
end
