ai.fsm = class('fsm')

function ai.fsm:init(super)
	self.super = super
end

function ai.fsm:detectEnemy(enemy)
	if self.engageDialog then
		self.super.space:createDialog( self.engageDialog, false )
		return true
	end

	if self.detectEnemyHandler then
		self.super:pushFunction( self.detectEnemyHandler(self.super, enemy) )
		return true
	end
	
	return false
end

function ai.fsm:enemyRoomAlert(enemy)
	if self.enemyRoomAlertHandler then
		self.super:pushFunction( self.enemyRoomAlertHandler(self.super, enemy) )
		return true
	end
	
	return false
end

function ai.fsm:detectBomb(bomb)
	if self.detectBombHandler then
		self.super:pushFunction( self.detectBomb(self.super, bomb) )
		return true
	end
	
	return false
end

function ai.fsm:zeroHP()
	if self.defeatDialog then
		self.super.space:createDialog( self.defeatDialog, false )
		return true
	end
	
	return false
end

function ai.fsm:addFleeBomb()
	self.detectBombHandler = ai.Flee.makeTargetFunctionGenerator(-1.0)
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

ai.blue_fairy_follow_path = ai.fsm:extend('blue_fairy_follow_path')

function ai.blue_fairy_follow_path:initialize()
	self.super:pushFunction( ai.ScriptFunction.create(self.super, "BlueFairy") )
end

ai.green_fairy = ai.fsm:extend('green_fairy')

function ai.green_fairy:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("GreenFairy")
	self:addFleeBomb()
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

ai.rumia1 = ai.fsm:extend('rumia1')

ai.rumia1.engageDialog = 'dialogs/rumia1'
ai.rumia1.defeatDialog = 'dialogs/rumia2'

function ai.rumia1:initialize()
	self.detectEnemyHandler = ai.ScriptFunction.targetGenerator("Rumia1")
end

ai.sakuya = ai.fsm:extend('sakuya')

function ai.sakuya:initialize()
	self.enemyRoomAlertHandler = ai.ScriptFunction.targetGenerator("SakuyaMain")
end

ai.sakuya_npc = ai.fsm:extend('sakuya_npc')

function ai.sakuya_npc:initialize()
	self.super:pushFunction( ai.ScriptFunction.create(self.super, "SakuyaNPC1") )
end
