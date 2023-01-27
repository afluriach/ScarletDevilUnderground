print("init script")

function loadArea(area, _start)
	local start = _start

	if not _start then
		start = 'player_start'
	end
	
	App.loadScene(area, start)
end

overworld = {}

function overworld.mansionB1()
	loadArea('mansion_b1', 'door2')
end

function overworld.graveyard()
	loadArea('graveyard', 'graveyard_entrance')
end

function overworld.forest()
	loadArea('forest', 'clearing_path')
end

function overworld.forest_lake()
	loadArea('forest_lake', 'forest_path')
end

function overworld.clearing_mansion()
	loadArea('clearing', 'mansion_door')
end

function overworld.clearing_graveyard()
	loadArea('clearing', 'to_graveyard')
end

function g1()
	loadArea('graveyard1')
end

function f1()
	loadArea('forest1')
end

function gotoDoor(d)
	App.getCrntSene():teleportToDoor(d)
end

function rumia1()
	gotoDoor('door-7b')
end
