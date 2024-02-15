objects.PressurePlate = class("PressurePlate")

function objects.PressurePlate:init(super, params)
	self.super = super
	self.contacts = 0

	if params.args.targets then
		self.targets = util.splitString(params.args.targets, " ")
	elseif params.args.target then
		self.targets = util.splitString(params.args.target, " ")
	end
	
	if not self.targets then
		app.log("PressurePlate:init: targets not defined!")
	end
end

function objects.PressurePlate:onContact(obj)
	if self.contacts == 0 then
		self:activateTargets()
	end
	self.contacts = self.contacts + 1
end

function objects.PressurePlate:onEndContact(obj)
	self.contacts = self.contacts - 1
	if self.contacts == 0 then
		self:deactivateTargets()
	end
end

function objects.PressurePlate:activateTargets()
	for i=1,#self.targets do
		local obj = self.super.space:getObjectByName(self.targets[i])
		if obj then
			obj:activate()
			app.log("activated " .. obj:getName())
		end
	end
end

function objects.PressurePlate:deactivateTargets(obj)
	for i=1,#self.targets do
		local obj = self.super.space:getObjectByName(self.targets[i])
		if obj then
			obj:deactivate()
			app.log("deactivated " .. obj:getName())
		end
	end
end
