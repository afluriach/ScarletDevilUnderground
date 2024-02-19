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

objects.TimedSwitch = class("TimedSwitch")

objects.TimedSwitch.margin = 1.0 / 6.0

function objects.TimedSwitch:init(super, params)
	self.super = super
	if params.args.activationTime then
		self.activationTime = tonumber(params.args.activationTime)
	end
	if params.args.activationType then
		self.activationType = GType[params.args.activationType]
	else
		app.log("Activation type not defined!")
	end
	self.numContacts = 0
	self.timeRemaining = 0.0
end

function objects.TimedSwitch:setColor(color)
	self.super:clearDrawNode()
	
	local leftEdge = -self.super.dim.x * 0.5;
	local rightEdge = self.super.dim.x * 0.5;
	local bottomEdge = -self.super.dim.y * 0.5;
	local topEdge = self.super.dim.y * 0.5;
	
	self.super:drawRectangle(
		Vec2.new(leftEdge, bottomEdge):scale(app_constants.pixelsPerTile),
		Vec2.new(rightEdge, topEdge):scale(app_constants.pixelsPerTile),
		Color4F.BLACK
	)
	
	self.super:drawRectangle(
		Vec2.new(leftEdge + self.margin, bottomEdge + self.margin):scale(app_constants.pixelsPerTile),
		Vec2.new(rightEdge - self.margin, topEdge - self.margin):scale(app_constants.pixelsPerTile),
		color
	)
end

function objects.TimedSwitch:initializeGraphics()
	self.super:createDrawNode(GraphicsLayer.floor)
	self:setColor(Color4F.RED)
end

function objects.TimedSwitch:update()
	if self.numContacts == 0 and self.timeRemaining > 0.0 and self.activationTime then
		self.timeRemaining = util.timerDecrement(self.timeRemaining)
		
		if self.timeRemaining <= 0.0 then
			self.super:deactivate()
			self:setColor(Color4F.RED)
		end
	end
end

function objects.TimedSwitch:onContact(obj)
	if not self.activationType or obj:getType() ~= self.activationType then return end

	if self.numContacts == 0 and self.activationTime then
		self.timeRemaining = self.activationTime
		self.super:activate()
		self:setColor(Color4F.CYAN)
	end
	self.numContacts = self.numContacts + 1
end

function objects.TimedSwitch:onEndContact(obj)
	if not self.activationType or obj:getType() ~= self.activationType then return end

	self.numContacts = self.numContacts - 1
	if self.numContacts == 0 then
		self:setColor(Color4F.GRAY)
	end
end
