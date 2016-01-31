
include "ian/ian.lua"
include "ian_server/ian_server.lua"


ian.shell.js:addFiles("license.js", "client-core.js", "shared/**.js", "client/**.js")

ian.shell.css:addFiles("license.css", "client/**.css")

local id = ian.deploy
local target2 = id.mergedTextFiles("client/test2.txt")
