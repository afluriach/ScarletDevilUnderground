function push_return(func)
	return ai.update_return.new(0, 0.0, func)
end

function pop_return()
	return ai.update_return.new(-1, 0.0, ai.Function.makeNullShared())
end

function steady_return(x)
	return ai.update_return.new(0, x, ai.Function.makeNullShared())
end
