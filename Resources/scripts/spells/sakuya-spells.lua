spells.IllusionDial = class('IllusionDial')

spells.IllusionDial.name = 'Illusion Dial'
spells.IllusionDial.description = ''

spells.IllusionDial.count = 16
spells.IllusionDial.radius = 2.5
spells.IllusionDial.arc_start = 0.0
spells.IllusionDial.arc_end = math.pi * 2.0
spells.IllusionDial.arc_width = spells.IllusionDial.arc_end - spells.IllusionDial.arc_start
spells.IllusionDial.arc_spacing = spells.IllusionDial.arc_width / (spells.IllusionDial.count - 1)
spells.IllusionDial.angular_speed = math.pi * 2.0 / 3.0
spells.IllusionDial.max_angle_margin = math.pi / 12.0
spells.IllusionDial.min_fire_interval = 1.0 / 3.0

function spells.IllusionDial.getParams()
	return spell_params.new(-1.0, 0.0, spell_cost.none())
end

function spells.IllusionDial:init(super)
	self.super = super
	self.agent = super:getCasterAsAgent()
	self.timeSinceLastFire = 0.0
	self.bullets = {}
	self.launch_flags = {}
	for i=1,self.count do
		self.launch_flags[i] = false
	end
end

function spells.IllusionDial:onEnter()
	self.targetRef = self.super:getSpace():getPlayerAsRef()
	self.props = app.getBullet('illusionDialDagger')
	
	for i=1,self.count do
		self.bullets[i] = self.agent:spawnBullet(
			self.props,
			SpaceVect.ray(self.radius, self.arc_start + i * self.arc_spacing),
			SpaceVect.new(),
			0.0,
			i % 2 ~= 0 and self.angular_speed or -self.angular_speed
		)
	end
end

function spells.IllusionDial:update()
	self.timeSinceLastFire = util.timerIncrement(self.timeSinceLastFire)
	
	if self.timeSinceLastFire > self.min_fire_interval then
		if self:tryLaunch() then
			self.timeSinceLastFire = 0.0
		end
		if self:allBulletsConsumed() then
			self.super:runEnd()
		end		
	end
end

function spells.IllusionDial:allBulletsConsumed()
	for i=1,self.count do
		if self.bullets[i]:isValid() and not self.launch_flags[i] then
			return false
		end
	end
	
	return true
end

function spells.IllusionDial:tryLaunch()
	local best = -1
	local best_angle = math.pi
	local target = self.targetRef:get()
	
	for i=1,self.count do
		if self.bullets[i]:isValid() and not self.launch_flags[i] then
			local crnt = ai.viewAngleToTarget(self.bullets[i]:get(),target)
			if crnt ~= math.huge and math.abs(crnt) < best_angle then
				best = i
				best_angle = math.abs(crnt)
			end
		end
	end
	
	if best ~= -1 and best_angle < self.max_angle_margin then
		self.bullets[best]:get():launchAtTarget(target)
		self.launch_flags[best] = true
		self.timeSinceLastFire = 0.0
	end
end

function spells.IllusionDial:onExit()
	for i,ref in ipairs(self.bullets) do
		if ref:isValid() and not self.launch_flags[i] then
			self.super:getSpace():removeObject(ref)
		end
	end
end
