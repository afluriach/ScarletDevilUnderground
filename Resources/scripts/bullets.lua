bullets = {}

bullets.starbowBreak = {}

bullets.starbowBreak.radiusScales = {
	0.75,
	0.5,
	2.0 / 3.0,
	0.25,
	1.0 / 3.0
}

function bullets.starbowBreak.addProps(idx)
	local sizeScale = bullets.starbowBreak.radiusScales[idx]
	local base = app.getBullet('starbowBullet')
	local crnt = app.addBullet('starbowBullet' .. idx, 'starbowBullet')
	
	crnt.speed = crnt.speed / sizeScale

	--struct fields are get and set by value, so they must be copied
	local dim = SpaceVect:new(crnt.dimensions)
	dim.x = dim.x * sizeScale
	crnt.dimensions = dim
	
	local damage = DamageInfo:new(crnt.damage)
	damage.mag = damage.mag * sizeScale
	crnt.damage = damage
	
	crnt.sprite = app.getSprite('starbowBreak' .. idx)
	crnt.light = app.getLight('starbowBreak' .. idx)
end

function loadBullets()
	for i, _v in ipairs(bullets.starbowBreak.radiusScales) do
		bullets.starbowBreak.addProps(i)
	end
end
