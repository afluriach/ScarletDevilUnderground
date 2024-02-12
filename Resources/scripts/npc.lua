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

objects.Patchouli2 = class("Patchouli")

objects.Patchouli2.numShelfs = 8
objects.Patchouli2.minWait = 3
objects.Patchouli2.maxWait = 6

function objects.Patchouli2:init(super)
	self.super = super
end

function objects.Patchouli2:initialize()
	self.points = {}
	
	for i=1,self.numShelfs do
		local name = 'bookshelf' .. i
		local bookshelf = self.super.space:getObject(name)
		
		table.insert(self.points, ai.getHorizontalAdjacentTiles(bookshelf))
	end
	
	self:waiting()
end

function objects.Patchouli2:moving()
	self.state = 'moving'
	self.func = nil
	
	while not self.func do
		self.func = ai.FollowPath.pathToPoint(
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
	local shelf = self.super.space:getRandomInt(1, self.numShelfs)
	local pv = self.points[shelf]
	return pv[self.super.space:getRandomInt(1,#pv)]
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
			self:waiting()
		else
			self:moving()
		end
	end

	self.func:update()
end
