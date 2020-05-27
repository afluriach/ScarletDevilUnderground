function wander(fsm)
	fsm:addThread(ai.Wander.create(fsm))
end

function maintain_distance(fsm)
	engage = ai.MaintainDistance.makeTargetFunctionGenerator(4.5, 1.5)
	fsm:addWhileDetectHandler(GType.player, engage)
end

--Used for large enemy (slimes) that should not try to use pathfinding to follow player
function engage_player_in_room(fsm)
	engage = ai.Seek.makeTargetFunctionGenerator(false, 0.0)
	fsm:addWhileDetectHandler(GType.player, engage)
end

function evade_player_projectiles(fsm)
	fsm:addThread(ai.Evade.create(fsm, GType.playerBullet))
end

function seek_player(fsm)
	engage = ai.Seek.makeTargetFunctionGenerator(true, 0.0)
	fsm:addWhileDetectHandler(GType.player, engage)
end

function flee_player(fsm)
	engage = ai.Flee.makeTargetFunctionGenerator(1.5)
	fsm:addWhileDetectHandler(GType.player, engage)
end

function fairy2(fsm)
	local engage = ai.ScriptFunction.targetGenerator("FairyEngage")
	fsm:addWhileDetectHandler(GType.player, engage)
end

function ghost_fairy(fsm)
	local engage = ai.ScriptFunction.targetGenerator("GhostFairyEngage")
	fsm:addWhileDetectHandler(GType.player, engage)
end

function ghost_fairy_npc(fsm)
	fsm:addThread(ai.Wander.create(fsm))
	
	seek = ai.Seek.makeTargetFunctionGenerator(true, 1.5)
	fsm:addWhileDetectHandler(GType.player, seek)
end

function green_fairy(fsm)
	fsm:addFleeBomb()
	fsm:addAlertHandler( ai.ScriptFunction.targetGenerator("GreenFairy") )
end

function blue_fairy_follow_path(fsm)
	fsm:addThread(ai.ScriptFunction.create(fsm, "BlueFairy"))
end

function red_fairy(fsm)
	fsm:addFleeBomb()

	local explode = ai.ExplodeOnZeroHP.create(
		fsm,
		DamageInfo.new(20.0, DamageType.bomb, Attribute.none, 100.0),
		4.0
	)
	fsm:addFunction(explode)
	
	local wander = ai.Wander.makeTargetFunctionGenerator(1.5, 2.5, 2.0, 3.0)
	fsm:addAlertHandler(wander)
	
	local engage = ai.ScriptFunction.targetGenerator("RedFairyEngage")
	fsm:addWhileDetectHandler(GType.player, engage)
end

function ice_fairy(fsm)
	local engage = ai.ScriptFunction.targetGenerator("FairyEngage")
	fsm:addWhileDetectHandler(GType.player, engage)
end

function zombie_fairy(fsm)
	local engage = ai.ScriptFunction.targetGenerator("ZombieFairy")
	fsm:addWhileDetectHandler(GType.player, engage)	
end

function bat(fsm)
	engage = ai.ScriptFunction.targetGenerator("BatEngage")
	fsm:addWhileDetectHandler(GType.player, engage)
end

function facer(fsm)
	fsm:addThread( ai.ScriptFunction.create(fsm, "Facer") )
end

function follower(fsm)
	fsm:addThread( ai.ScriptFunction.create(fsm, "Follower") )
end

function forest_marisa(fsm)
	fsm:addAlertHandler( ai.ScriptFunction.targetGenerator("MarisaForestMain") )
end

function patchouli_enemy(fsm)	
	fsm:addAlertHandler( ai.ScriptFunction.targetGenerator("PatchouliEnemy") )
end

function reimu_enemy(fsm)
	local boss = ai.BossFightHandler.create(
		fsm,
		'dialogs/reimu_forest_pre_fight',
		'dialogs/reimu_forest_post_fight'
	)
	local engage = ai.ScriptFunction.targetGenerator("ReimuEnemy")
	
	fsm:addFunction(boss)
	fsm:addOnDetectHandler(GType.player, engage)
end

function rumia1(fsm)
	local engage = ai.ScriptFunction.targetGenerator("Rumia1")
	local boss = ai.BossFightHandler.create(fsm, 'dialogs/rumia1', 'dialogs/rumia2')
	
	fsm:addFunction(boss)
	fsm:addOnDetectHandler(GType.player, engage)
end

function sakuya(fsm)
	fsm:addAlertHandler( ai.ScriptFunction.targetGenerator("SakuyaMain") )
end

function sakuya_npc(fsm)
	fsm:addThread( ai.ScriptFunction.create(fsm, "SakuyaNPC1") )
end

function scorpion1(fsm)
	engage = ai.Scurry.makeTargetFunctionGenerator(3.0, -1.0)
	fsm:addWhileDetectHandler(GType.player, engage)
end

function scorpion2(fsm)
	engage = ai.Flank.makeTargetFunctionGenerator(3.0, -1.0)
	fsm:addWhileDetectHandler(GType.player, engage)
end

function stalker(fsm)
	fsm:addThread( ai.ScriptFunction.create(fsm, "StalkerTeleport") )

	engage = ai.Seek.makeTargetFunctionGenerator(true, 0.0)
	fsm:addWhileDetectHandler( GType.player, engage )
	
	fsm.agent:applyMagicEffect(
		app.getEffect("DrainStaminaFromMovement"),
		effect_attributes.new(1.0, -1.0)
	)	
end
