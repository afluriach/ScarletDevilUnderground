objects.Enemy = class("Enemy")

--objects.Enemy.ai_state = {
--   none = 1,
--   idle = 2,
--   detect = 3,
--   surprise = 4,
--   engage = 5,
--   flee = 6,
--}

function objects.Enemy:init(super)
	self.super = super
	self.state = 'none'
end

function objects.Enemy:setState(newState)
	app.log(string.format("%s enters %s state", self.super:toString(), newState))
	
	if self.state ~= 'none' then
		local exit = self[self.state .. '_exit']
		if exit then
			exit(self)
		end
	end

	self.state = newState
	
	if self.state ~= 'none' then
		local enter = self[self.state .. '_enter']
		if enter then
			enter(self)
		end
	end
end

function objects.Enemy:initialize()
	self:setState('idle')
end

function objects.Enemy:update()
	if self.state ~= 'none' then
		self.func:runUpdate()
		local s = self[self.state]
		if s then s(self) end
	end
end

function objects.Enemy:onDetectEnemy(target)
	self.target = target
	
	if self.state == 'engage' or self.state == 'flee' then return end
	
	if self.engageImmediately then
		self:setState('engage')
	else
		self:setState('detect')
	end
end

function objects.Enemy:onHit()
	if not self.target then return end
	if
		self.hpFleeThreshold and
		self.super:get(Attribute.hpRatio) < self.hpFleeThreshold and
		self.state ~= 'flee'
	then
		self:setState('flee')
	elseif self.state ~= 'engage' and self.engageOnHit then
		self:setState('engage')
	end
end

--function objects.Enemy:onZeroHP()
--	self.super.space:removeObject(self.super:getAsObject())
--end
--
--function objects.Enemy:onPitfall()
--	self.super.space:removeObject(self.super:getAsObject())
--end

--------------------------------------------------------------------------------

objects.Bat = objects.Enemy:extend('Bat', {
	engageOnHit = true,
	touchSurprise = true,
	hpFleeThreshold = 0.3,
	idle_enter = function(self)
		self.func = ai.Wander.create(self.super:getAsObject(), 1.0, 4.0, 2.0, 4.0)
	end,
	detect_enter = function(self)
		self.func = ai.Wander.create(self.super:getAsObject(), 0.5, 1.5, 1.0, 2.0)
	end,
	engage_enter = function(self)
		self.func = ai.Flank.create(self.super:getAsObject(), self.target, 2.0, 1.0)
	end,
	flee_enter = function(self)
		self.func = ai.Flee.create(self.super:getAsObject(), self.target, 6.0)
	end
})

--------------------------------------------------------------------------------

objects.CathedralFairy = class('CathedralFairy')

function objects.CathedralFairy:init(super)
	self.super = super
end

function objects.CathedralFairy:seek()
	self.moveFunc = ai.Seek.create(self.super:getAsObject(), self.target, true, 0.0)
end

function objects.CathedralFairy:onDetectEnemy(target)
	self.target = target
end

function objects.CathedralFairy:update()
	if self.moveFunc and not self.moveFunc:isCompleted() then
		self.moveFunc:runUpdate()
	elseif self.target then
		self:seek()
	end
end
