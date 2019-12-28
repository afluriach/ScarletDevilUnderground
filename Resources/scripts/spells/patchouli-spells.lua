spells.Whirlpool1 = class('Whirlpool1', {
	name = 'Whirlpool',
	description = '',
	shotInterval = 0.25,
	angularSpeed = math.pi / 6.0,
	angularOffset = math.pi / 12.0,
	getParams = function()
		return spell_params.new(-1.0, 0.0, spell_cost.none())
	end,
	init = function(self, super)
		self.super = super
		self.agent = super:getCasterAsAgent()
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
				angle + agentAngle
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
	getParams = function()
		return spell_params.new(-1.0, 0.0, spell_cost.none())
	end,
	init = function(self, super)
		self.super = super
		self.agent = super:getCasterAsAgent()
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
				angle + agentAngle
			)
		end
		
		self.shotTimer = self.shotInterval
	end
})
