effects.FreezeStatus = class('FreezeStatus')

function effects.FreezeStatus.getFlags()
	return MagicEffect.make_flags_bitfield(MagicEffect_flags.timed)
end

function effects.FreezeStatus:init(target)
    self.target = target:getAsAgent()
end

function effects.FreezeStatus:onEnter()
    self.target:addGraphicsAction(graphics.freezeEffectAction(), 0)
    self.target:setFrozen(true)
    self.target:setFiringSuppressed(true)
    self.target:setMovementSuppressed(true)
end

function effects.FreezeStatus:onExit()
    self.target:addGraphicsAction(graphics.freezeEffectEndAction(), 0)
    self.target:setFrozen(false)
    self.target:setFiringSuppressed(false)
    self.target:setMovementSuppressed(false)
end
