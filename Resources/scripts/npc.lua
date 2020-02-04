objects.BlueFairyNPC = class("BlueFairyNPC")

function objects.BlueFairyNPC.conditionalLoad(space,attrs,props)
	return attrs.level > App.getCrntState():getAttribute("BlueFairyLevel")
end

function objects.BlueFairyNPC:init(super)
	self.super = super
end

function objects.BlueFairyNPC:correct_order(idx)
	app.log("Level: " .. self.super:getLevel())
	return idx == self.super:getLevel() and idx == App.getCrntState():getAttribute("BlueFairyLevel") + 1	
end

function objects.BlueFairyNPC:satisfy_condition()
	return  App.getCrntState():getAttribute("mushroomCount") >= self.super:getLevel() and
	self.super:getLevel() == App.getCrntState():getAttribute("BlueFairyLevel") + 1
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

	app.log("satisfy_effect" .. self.super:getName())

	App.getCrntState():incrementAttribute("BlueFairyLevel")
	App.getCrntState():subtractAttribute("mushroomCount", self.super:getLevel())
	
	self.super:getSpace():removeObject(self.super:asGObject())
end

function objects.BlueFairyNPC:initialize()

	app.log("BlueFairy:initialize ")
	
end

objects.GhostFairyNPC = class("GhostFairyNPC")

function objects.GhostFairyNPC.conditionalLoad(space,attrs,props)	
	if attrs.level > 0 then
		return App.getCrntState():isChamberCompleted('graveyard' .. attrs.level)
	else
		return true
	end
end

objects.Meiling1 = class("Meiling1")

function objects.Meiling1.conditionalLoad(space,attrs,props)
	return not App.getCrntState():isChamberCompleted("graveyard3")
end
