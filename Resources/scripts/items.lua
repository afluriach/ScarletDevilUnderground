objects.Collectible = class('Collectible')

objects.Collectible.info = {
	health1 = {'RestoreHP', 5.0, 0.0},
	health2 = {'RestoreHP', 25.0, 0.0},
	health3 = {'RestoreHP', 100.0, 0.0},
	magic1 = {'RestoreMP', 5.0, 0.0},
	magic2 = {'RestoreMP', 25.0, 0.0},
	magic3 = {'RestoreMP', 100.0, 0.0},
	speed1 = {'FortifyAgility', 1.0, 15.0},
	speed2 = {'FortifyAgility', 2.0, 60.0},
}

function objects.Collectible:init(super)
	self.super = super
	local info = self.info[super:getClsName()]

	if not info then
		app.log('Unknown Collectible type ' .. super:getClsName())
	else
		self.effect = app.getEffect(info[1])
		self.attrs = effect_attributes.new( info[2], info[3] )
	end
end

function objects.Collectible:canAcquire(player)
	return self.effect:canApply(player, self.attrs)
end

function objects.Collectible:onAcquire(player)
	player:applyMagicEffect(self.effect, self.attrs)
	self.super.space:removeObject(self.super)
	self.super:playSoundSpatial('sfx/powerup.wav', 1.0, false, 0.0)
end

objects.MapFragment = class('MapFragment')

objects.MapFragment.fragments = {
	g3_map1 = {0,1,3,5}
}

function objects.MapFragment:init(super)
	self.super = super
end

function objects.MapFragment:onAcquire(p)
	local f = self.fragments[self.super:getName()]
	
	if not f then
		app.log('Unknown map fragment: ' .. self.super:getName())
		return
	end
	
	for _i,v in ipairs(f) do
		self.super.space:registerRoomMapped(v)		
	end
end

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

objects.Upgrade = class('Upgrade')

objects.Upgrade.info = {
	hp_upgrade = { Attribute.maxHP, 25.0 },
	mp_upgrade = { Attribute.maxMP, 25.0 },
	stamina_upgrade = { Attribute.maxStamina, 25.0 },
	agility_upgrade = { Attribute.agility, 1.0 },
	attack_upgrade = { Attribute.attack, 0.25 },
	attack_speed_upgrade = { Attribute.attackSpeed, 0.25 },
	shield_upgrade = { Attribute.shieldLevel, 0.25 },
	bullet_speed_upgrade = { Attribute.bulletSpeed, 0.5 }
}

function objects.Upgrade:init(super)
	self.super = super
end

function objects.Upgrade:onAcquire(p)
	local u = self.info[self.super:getClsName()]	

	if not u then
		app.log("Unknown upgrade type: " .. self.super:getClsName())
		return
	end
	
	p:applyUpgrade(u[1], u[2])
end
