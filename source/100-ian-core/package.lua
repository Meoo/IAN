
include "ian/ian.lua"
include "ian_server/ian_server.lua"


ian.shell.js:addFiles("client-core.js", "shared/**.js", "client/**.js")

ian.shell.css:addFiles("client/**.css")
