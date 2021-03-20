objects.BlueFairyNPC = class("BlueFairyNPC")

function objects.BlueFairyNPC:init(super)
	self.super = super
end

function objects.BlueFairyNPC:correct_order(idx)
	return idx == self.super.level and idx == App.state:getAttribute("BlueFairyLevel") + 1	
end

function objects.BlueFairyNPC:satisfy_condition()
	return  App.state:getItemCount("Mushroom") >= self.super.level and
	self.super.level == App.state:getAttribute("BlueFairyLevel") + 1
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
	App.state:removeItem("Mushroom", self.super.level)
	
	self.super.space:removeObject(self.super:asGObject())
end

function objects.BlueFairyNPC:initialize()

	app.log("BlueFairy:initialize ")
	
end
