
mw.project "Server"
  kind "ConsoleApp"

  includedirs { "." }
  files { "**.cpp", "shared/**.cpp" }

  import {
    ["mozjs"] = { "default" },
    ["websocketpp"] = { "tls" },

    --["sfml2"] = { "graphics" },
  }
