-- water.lua
-- adding a water block via lua module

water_block_id = 0;

function startup()
	io.write("adding a 'water' block\n");

	water_block_id = api.add_block_type('water');
end

function shutdown()
	io.write("water module shutting down\n");
end

function after_terrain_generated()
	io.write("fires after a block of terrain is generated");
end
