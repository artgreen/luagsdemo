md = require "memdump"
md:initlize()
md:dump( "dump.log" )

-- Get the ticks of the status object
ticks = status.getTicks()
print("Initial ticks:", ticks)

-- Set the ticks of the status object
status.setTicks(42)

-- Get the new ticks of the status object
ticks = status.getTicks()
print("New ticks:", ticks)

-- Get the name of the status object
name = status.getName()
print("Initial name:", name)

-- Set the name of the status object
status.setName("New status")

-- Get the new name of the status object
name = status.getName()
print("New name:", name)

