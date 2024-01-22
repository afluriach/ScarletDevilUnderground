objects.Barrier = class('Barrier')

function objects.Barrier:init(super)
	self.super = super
end

function objects.Barrier:initialize(super)
	self:setSealed(self.super.active)
end

--Show sprite and unset sensor property when activated.
function objects.Barrier:setSealed(val)
	self.super:setBodySensor(not val)
	self.super:setSpriteVisible(val)
end

function objects.Barrier:onActivate(val)
	self:setSealed(true)
end

function objects.Barrier:onDeactivate(val)
	self:setSealed(false)
end

objects.Desk = class('Desk')

function objects.Desk:init(super)
	self.super = super
end

function objects.Desk:interact(p)
	self.super.space:enterWorldSelect()
end

objects.DestructibleHeadstone = class('DestructibleHeadstone')

objects.DestructibleHeadstone.damageSprites = {
	{0, 25, "sprites/headstone-damage3.png"},
	{25, 50, "sprites/headstone-damage2.png"},
	{50, 75, "sprites/headstone-damage1.png"},
	{75, 100, "sprites/headstone.png"}
}

function objects.DestructibleHeadstone:init(super)
	self.super = super
	self.crntSprite = objects.DestructibleHeadstone.damageSprites[4][3]
	self.hp = 100
end

function objects.DestructibleHeadstone:updateSprite()
	for i,v in ipairs(self.damageSprites) do
		if self.hp >= v[1] and self.hp < v[2] and self.crntSprite ~= v[3] then
			self.super:setSpriteTexture(v[3])
			self.crntSprite  = v[3]
			return
		end
	end
end

function objects.DestructibleHeadstone:hit(damage, n)
	self.hp = self.hp - damage.mag
	
	if self.hp <= 0.0 then
		self.super.space:removeObject(self.super:asGObject())
	else
		self:updateSprite()
	end
end

objects.Goal = class('Goal')

objects.Goal.soundInterval = 0.15
objects.Goal.soundGain = 0.5

function objects.Goal:init(super)
	self.super = super
	self.soundTimer = self.soundInterval
end

function objects.Goal:update(super)
	self.soundTimer = util.timerDecrement(self.soundTimer)
	
	if self.soundTimer <= 0.0 then
		self.super:playSoundSpatial('sfx/shot.wav', self.soundGain, false, 0.0)
		self.soundTimer = self.soundInterval
	end
end

function objects.Goal:canInteract(p)
	return not self.super.active
end

function objects.Goal:interact(p)
	self.super.space:triggerSceneCompleted()
end

objects.GhostHeadstone = class('GhostHeadstone')

function objects.GhostHeadstone:init(super, args)
	self.super = super
	self.fairies = hashset_gobject_ref.new()
	self.cost = util.getIntOrDefault(args, "cost", 0)
end

function objects.GhostHeadstone:applyRemoval()
	self.fairies:for_each(function(ref)
		if ref:isValid() then
			self.super.space:removeObject(ref)
		end
	end)
	self.fairies:clear()
end

function objects.GhostHeadstone:checkActivate()
	if self.fairies:size() >= self.cost then
		self:applyRemoval()
		self.super.space:removeObject(self.super:getRef())
	end
end

function objects.GhostHeadstone:onContact(obj)
	if obj:getType() == GType.player then
		self:checkActivate()
	elseif obj:getType() == GType.npc and obj:getClsName() == 'GhostFairyNPC' then
		local ref = obj:getRef()
		self.fairies:insert(ref)
		self:checkActivate()
	end
end

function objects.GhostHeadstone:onEndContact(obj)
	if obj:getType() == GType.npc and obj:getClsName() == 'GhostFairyNPC' then
		self.fairies:erase(obj:getRef())
	end
end

function objects.GhostHeadstone:initialize()	
	self.sensor = self.super.space:createAreaSensor(
		self.super.space:getArea(self.super.name),
		GType.player | GType.npc,
		function(obj) self:onContact(obj) end,
		function(obj) self:onEndContact(obj) end
	)
end

function objects.GhostHeadstone:onRemove()
	if self.sensor then
		self.super.space:removeObject(self.sensor)
	end
end

objects.Launcher = class('Launcher')

objects.Launcher.fireInterval = 0.25

function objects.Launcher:init(super)
	self.super = super
	self.cooldownTime = 0.0
end

function objects.Launcher:update()
	self.cooldownTime = util.timerDecrement(self.cooldownTime)
	
	if self.super.active and self.cooldownTime <= 0.0 then
		local pos = self.super:getPos()
		pos = pos + SpaceVect.ray(1.0, self.super:getAngle())
		
		self.super.space:createBullet(
			Bullet.makeParams(pos, self.super:getAngle(), SpaceVect.new(), 0.0, SpaceVect.new()),
			bullet_attributes.getDefault(),
			app.getBullet('launcherBullet')
		)
		self.cooldownTime = self.fireInterval
	end
end

objects.Sign = class('Sign')

function objects.Sign:init(super)
	self.super = super
end

function objects.Sign:interact(p)
	self.super.space:createDialog('dialogs/' .. self.super.name, false)
end

objects.Torch = class('Torch')

function objects.Torch:init(super)
	self.super = super
end

function objects.Torch:interact(p)
	self.super:toggleActive()
end

function objects.Torch:initializeGraphics()
	self.flameSpriteID = self.super.space:createSprite(
		"white_flame",
		GraphicsLayer.overhead,
		self.super:getPos(),
		0.5
	)
	self.super.space:setSpriteVisible(self.flameSpriteID, self.super:getActive())
	
	if not self.super:getActive() then
		self.super:removeLight()
	end	
end

function objects.Torch:spriteFadeIn()
	self.super.space:setSpriteVisible(self.flameSpriteID, true)
end

function objects.Torch:spriteFadeOut()
	self.super.space:setSpriteVisible(self.flameSpriteID, false)
end

function objects.Torch:onActivate()
	self.super.space:setSpriteVisible(self.flameSpriteID, true)
	self.super:createLight()
end

function objects.Torch:onDeactivate()
	self.super.space:setSpriteVisible(self.flameSpriteID, false)
	self.super:removeLight()
end
