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
			Bullet.makeParams(pos, self.super:getAngle()),
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
	self.super.space:createDialog('dialogs/' .. self.super:getName(), false)
end
