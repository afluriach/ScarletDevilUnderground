spells.YinYangOrbs = class('YinYangOrbs')

spells.YinYangOrbs.orbCount = 4

function spells.YinYangOrbs.getParams()
	return spell_params.new(-1.0, -1.0, spell_cost.none())
end

function spells.YinYangOrbs:init(super)
	self.super = super
	self.agent = self.super.agent
end

function spells.YinYangOrbs:onEnter()
	self.orbs = {}
	local props = app.getBullet('yinYangOrb')

	for i=1,self.orbCount do
		local angle = math.pi * (0.25 + (i-1)*0.5)
		self.orbs[i] = self.agent:launchBullet(
			props,
			SpaceVect.ray(1.5, angle),
			angle,
			math.pi,
			false
		)
	end
end

function spells.YinYangOrbs:onExit()
	for _i,v in ipairs(self.orbs) do
		self.super.space:removeObject(v)
	end
end
