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
	self.bullets = gobject_ref_unordered_set()
end

function spells.IllusionDial:onEnter()
	self.targetRef = self.super:getSpace():getPlayerAsRef()
	self.props = app.getBullet('illusionDialDagger')
	
	for i=0,self.count-1 do
		local ref = self.agent:spawnBullet(
			self.props,
			SpaceVect.ray(self.radius, self.arc_start + i * self.arc_spacing),
			SpaceVect.new(),
			0.0,
			i % 2 ~= 0 and self.angular_speed or -self.angular_speed
		)
		self.bullets[ref] = true
	end
end

function spells.IllusionDial:update()
	self.timeSinceLastFire = util.timerIncrement(self.timeSinceLastFire)
	
	if self.timeSinceLastFire > self.min_fire_interval then
		if self:tryLaunch() then
			self.timeSinceLastFire = 0.0
		end
		if self:allBulletsConsumed() then
			self.super:stop()
		end		
	end
end

function spells.IllusionDial:allBulletsConsumed()
	return self.bullets:size() == 0
end

function spells.IllusionDial:tryLaunch()
	local best
	local best_angle = math.pi
	local target = self.targetRef:get()
	
	for _i, ref in pairs(self.bullets) do
		if ref:isValid() then
			local crnt = ai.viewAngleToTarget(ref:get(),target)
			if crnt ~= math.huge and math.abs(crnt) < best_angle then
				best = ref
				best_angle = math.abs(crnt)
			end
		end
	end
	
	if best and best_angle < self.max_angle_margin then
		best:get():launchAtTarget(target)
		self.bullets[best] = nil
		self.timeSinceLastFire = 0.0
	end
end

function spells.IllusionDial:onExit()
	for _i,ref in pairs(self.bullets) do
		if ref:isValid() then
			self.super:getSpace():removeObject(ref)
		end
	end
end
