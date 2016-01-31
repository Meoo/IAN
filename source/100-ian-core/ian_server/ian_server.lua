
ian.project "Server"
  kind "ConsoleApp"

  -- Only enable the project when building for C
  if IAN_JS then virtualproject(true) end

  includedirs { "." }
  files { "**.cpp" }

  import {
    ["IAN"] = { "default" },

    ["mozjs"] = { "default" },
    ["websocketpp"] = { "tls" },

    --["sfml2"] = { "graphics" },
  }
