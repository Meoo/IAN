
mw.project "Server"
  kind "ConsoleApp"

  includedirs { "." }
  files { "**.cpp", "shared/**.cpp" }

  import {
    ["mozjs"] = { "default" },
    ["websocketpp"] = { "default" },

    ["sfml2"] = { "graphics" },
  }
