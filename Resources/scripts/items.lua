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
