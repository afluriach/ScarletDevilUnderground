spells.FireStarburst = class('FireStarburst', {
	name = 'Fire Starburst',
	description = '',
	length = -1.0,
	updateInterval = 0.5,
	cost = spell_cost.none(),
	init = function(self, super)
		self.super = super
		self.props = app.getBullet('fireStarburstBullet')
		self.agent = self.super.agent
	end,
	update = function(self)
		for i=0,7 do
			local angle = math.pi * i * 0.25
			local pos = SpaceVect.ray(1.0, angle)
			
			self.agent:launchBullet(self.props, pos, angle, 0.0, true)
		end
	end
})

spells.FlameFence = class('FlameFence', {
	name = 'Flame Fence',
	description = '',
	length = -1.0,
	updateInterval = -1.0,
	cost = spell_cost.none(),
	init = function(self, super)
		self.bullets = {}
		self.super = super
		self.props = app.getBullet('fireFenceBullet')
		self.agent = self.super.agent
	end,
	onEnter = function(self)
		for y=-10,10,2 do
			local rowSkew = SpaceVect.new( y % 2 ~= 0 and 0.5 or 0.0, 0.0 )
			
			for x=-10,10,2 do
				local pos = SpaceVect.new(x,y) + rowSkew
				local ref = self.agent:spawnBullet(
					self.props,
					pos,
					SpaceVect.new(),
					0.0,
					0.0
				)
				self.bullets[ref] = true
			end
		end
	end,
	onExit = function(self)
		for ref, _v in pairs(self.bullets) do
			if ref:isValid() then
				self.super.space:removeObject(ref)
			end
		end
	end
})

spells.Whirlpool1 = class('Whirlpool1', {
	name = 'Whirlpool',
	description = '',
	shotInterval = 0.25,
	angularSpeed = math.pi / 6.0,
	angularOffset = math.pi / 12.0,
	length = -1.0,
	updateInterval = 0.0,
	cost = spell_cost.none(),
	init = function(self, super)
		self.super = super
		self.agent = self.super.agent
		self.bulletProps = app.getBullet('waterBullet1')
		self.shotTimer = 0.0
		
		self.angles = {
			-self.angularOffset,
			0.0,
			self.angularOffset,
			math.pi - self.angularOffset,
			math.pi,
			math.pi + self.angularOffset
		}
	end,
	onEnter = function(self)
		self.agent:setAngularVel(self.angularSpeed)
	end,
	update = function(self)
		self.shotTimer = util.timerDecrement(self.shotTimer)
		if self.shotTimer > 0.0 then return end
		local agentAngle = self.agent:getAngle()
	
		for _i,angle in ipairs(self.angles) do
			self.agent:launchBullet(
				self.bulletProps,
				SpaceVect.ray(1.0, angle + agentAngle),
				angle + agentAngle,
				0.0,
				true
			)
		end
		
		self.shotTimer = self.shotInterval
	end
})

spells.Whirlpool2 = class('Whirlpool2', {
	name = 'Whirlpool II',
	description = '',
	shotInterval = 1.0 / 6.0,
	angularSpeed = math.pi / 5.0,
	angularOffset = math.pi / 10.0,
	length = -1.0,
	updateInterval = 0.0,
	cost = spell_cost.none(),
	init = function(self, super)
		self.super = super
		self.agent = self.super.agent
		self.bulletProps = app.getBullet('waterBullet2')
		self.shotTimer = 0.0
		
		self.angles = {
			-self.angularOffset,
			0.0,
			self.angularOffset,
			math.pi*0.5 - self.angularOffset,
			math.pi*0.5,
			math.pi*0.5 + self.angularOffset,
			math.pi - self.angularOffset,
			math.pi,
			math.pi + self.angularOffset,
			math.pi*1.5 - self.angularOffset,
			math.pi*1.5,
			math.pi*1.5 + self.angularOffset
		}
	end,
	onEnter = function(self)
		self.agent:setAngularVel(self.angularSpeed)
	end,
	update = function(self)
		self.shotTimer = util.timerDecrement(self.shotTimer)
		if self.shotTimer > 0.0 then return end
		local agentAngle = self.agent:getAngle()
	
		for _i,angle in ipairs(self.angles) do
			self.agent:launchBullet(
				self.bulletProps,
				SpaceVect.ray(1.0, angle + agentAngle),
				angle + agentAngle,
				0.0,
				true
			)
		end
		
		self.shotTimer = self.shotInterval
	end
})
