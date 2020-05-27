spells.StarlightTyphoon = class('StarlightTyphoon', {
	name = 'Starlight Typhoon',
	description = '',
	length = 1.0,

	width = math.pi * 0.25,
	offset = 0.7,
	count = 30,
	bulletNames = {
		"starBulletBlue",
		"starBulletGreen",
		"starBulletGrey",
		"starBulletIndigo",
		"starBulletPurple",
		"starBulletRed",
		"starBulletYellow"
	},
	init = function(self, super)
		self.super = super
		self.agent = self.super.agent
		self.angle = self.agent:getAngle()
		self.shotsPerFrame = self.count / self.length * App.getParams().secondsPerFrame
		self.accumulator = 0.0
		
		self.bullets = {}
		for i,name in ipairs(self.bulletNames) do
			self.bullets[i] = app.getBullet(name)
		end
	end,
	fire = function(self)
		local space = self.super.space
		
		local arcPos = space:getRandomFloat(-self.width, self.width) + self.agent:getAngle()
		local pos = self.agent:getPos() + SpaceVect.ray(self.offset, self.angle)

		local props = self.bullets[space:getRandomInt(1,#self.bullets)]
		local attrs = self.agent:getBulletAttributes(props)
		attrs.size = space:getRandomFloat(0.7, 1.3)
		attrs.bulletSpeed = attrs.bulletSpeed * space:getRandomFloat(0.5, 1.5)
		local params = Bullet.makeParams(pos, arcPos, SpaceVect.new(), 0.0, props.dimensions:scale(attrs.size))
		
		space:createBullet(params,attrs,props)
	end,
	update = function(self)
		self.accumulator = self.accumulator + self.shotsPerFrame
		while self.accumulator >= 1.0 do
			self:fire()
			self.accumulator = self.accumulator - 1
		end
	end
})

