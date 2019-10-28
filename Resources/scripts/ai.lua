function push_return(func)
	return ai.update_return.new(0, func)
end

function pop_return()
	return ai.update_return.new(-1, ai.Function.makeNullShared())
end

function steady_return()
	return ai.update_return.new()
end
