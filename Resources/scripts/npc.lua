objects.BlueFairyNPC = class("BlueFairyNPC")

function objects.BlueFairyNPC:init(super)
	self.super = super
	self.id = util.getIntSuffix(self.super.name)
end

function objects.BlueFairyNPC:correct_order(idx)
	return idx == self.id and idx == App.state:getAttribute("BlueFairyLevel") + 1	
end

function objects.BlueFairyNPC:satisfy_condition()
	return  App.state:getItemCount("Mushroom") >= self.id and
	self.id == App.state:getAttribute("BlueFairyLevel") + 1
end

function objects.BlueFairyNPC:request_condition1()
	return self:correct_order(1)
end

function objects.BlueFairyNPC:request_condition2()
	return self:correct_order(2)
end

function objects.BlueFairyNPC:request_condition3()
	return self:correct_order(3)
end

function objects.BlueFairyNPC:satisfy_effect()

	app.log("satisfy_effect" .. self.super.name)

	App.state:incrementAttribute("BlueFairyLevel")
	App.state:removeItem("Mushroom", self.id)
	
	self.super.space:removeObject(self.super:getAsObject())
end

function objects.BlueFairyNPC:initialize()

	app.log("BlueFairy:initialize ")
	
end

--------------------------------------------------------------------------------

objects.MeilingGuard = class("MeilingGuard")

objects.MeilingGuard.states = {
	meiling1 = 'look_around',
	meiling2 = 'loop_path',
	meiling3 = 'scan_path',
	meiling4 = 'scan_path',
	meiling5 = 'scan_path',
	meiling6 = 'scan_path',
	meiling7 = 'loop_path'
}

objects.MeilingGuard.angular_speed = math.pi * 0.5

function objects.MeilingGuard:init(super)
	self.super = super
end

function objects.MeilingGuard:initialize(super)
	if self.super.name == '' then
		app.log('Un-named guard!')
		return
	end

	local s = self.states[self.super.name]
	if not s then
		app.log(string.format("Guard %s does not have a state defined.", self.super.name))
		return
	end
	
	local f = self[s]
	if not f then
		app.log(string.format("Unknown state function %s!", s))
		return
	end
	
	f(self)
end

function objects.MeilingGuard:update(super)
	if self.func then
		self.func:runUpdate()
	end
end

function objects.MeilingGuard:look_around(super)
	self.func = ai.LookAround.create(self.super:getAsObject(), self.angular_speed)
end

function objects.MeilingGuard:loop_path(super)
	local p = self.super.space:getPath(self.super.name)
	if not p then
		app.log(string.format("Unknown path %s!", self.super.name))
		return
	end
	
	self.func = ai.FollowPathKinematic.create(
		self.super:getAsObject(),
		p,
		ai.follow_path_mode.loop
	)
end

--function objects.MeilingGuard:scan_path(super)
--end

--------------------------------------------------------------------------------

objects.Patchouli2 = class("Patchouli2")

function objects.Patchouli2:init(super)
	self.super = super
end

function objects.Patchouli2:initialize()
	self.pointsA = {}
	self.pointsB = {}
	
	for i=1,self.numShelfs do
		local name = self.target .. i
		local bookshelf = self.super.space:getObject(name)
		
		table.insert(self.pointsA, util.getAdjacentTiles(bookshelf, self.direction))
		table.insert(self.pointsB, util.getAdjacentTiles(bookshelf, util.invertDirection(self.direction)))
	end
	
	self:waiting()
end

function objects.Patchouli2:moving()
	self.state = 'moving'
	self.func = nil
	
	if self.super.space:getRandomBool() then
		self.dir = self.direction
		self.pv = self.pointsA
	else
		self.dir = util.invertDirection(self.direction)
		self.pv = self.pointsB
	end
	
	while not self.func do
		self.func = ai.FollowPathKinematic.pathToPoint(
			self.super:getAsObject(),
			self:getRandomPoint()
		)
	end
end

function objects.Patchouli2:waiting()
	self.state = 'waiting'
	self.func = ai.Wait.create(
		self.super:getAsObject(),
		self:getRandomWait()
	)
end

function objects.Patchouli2:getRandomPoint()
	local shelfIdx = self.super.space:getRandomInt(1, self.numShelfs)
	local shelf = self.pv[shelfIdx]
	return shelf[self.super.space:getRandomInt(1,#shelf)]
end

function objects.Patchouli2:getRandomWait()
	return self.super.space:getRandomFloat(
		self.minWait,
		self.maxWait
	)
end

function objects.Patchouli2:update()
	
	if self.func:isCompleted() then
		if self.state == 'moving' then
			self.super:setDirection(util.invertDirection(self.dir))
			self:waiting()
		else
			self:moving()
		end
	end

	self.func:runUpdate()
end

objects.Patchouli2A = objects.Patchouli2:extend("Patchouli2A")
objects.Patchouli2A.target = "bookshelfA"
objects.Patchouli2A.numShelfs = 8
objects.Patchouli2A.minWait = 3.0
objects.Patchouli2A.maxWait = 6.0
objects.Patchouli2A.direction = Direction.right

objects.Patchouli2B = objects.Patchouli2:extend("Patchouli2B")
objects.Patchouli2B.target = "bookshelfB"
objects.Patchouli2B.numShelfs = 9
objects.Patchouli2B.minWait = 1.0
objects.Patchouli2B.maxWait = 10.0
objects.Patchouli2B.direction = Direction.up

objects.Patchouli2C = objects.Patchouli2:extend("Patchouli2C")
objects.Patchouli2C.target = "bookshelfC"
objects.Patchouli2C.numShelfs = 3
objects.Patchouli2C.minWait = 0.333
objects.Patchouli2C.maxWait = 0.666
objects.Patchouli2C.direction = Direction.up
