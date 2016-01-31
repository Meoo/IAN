
ian.project "IAN"
  kind "StaticLib"

  -- Only enable the project when building for C
  if IAN_JS then virtualproject(true) end

  includedirs { "include" }
  files { "src/**.cpp" }

  export "*"
    links "IAN"
