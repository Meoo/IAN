
require "scripts/ian"
require "scripts/mwb"


ian.solution "IAN"
  language "C++"

  -- HELPERS
  defines { [[__USER__="\"]].. (os.getenv("USERNAME") or "Unknown"):gsub("[\\\"]", "?") ..[[\""]] }

  -- COMPILER SETTINGS
  rtti    "Off"
  flags   { "C++11" }

  -- Ignore warnings on Visual Studio
  filter { "action:not vs*" }
    flags   { "ExtraWarnings", "FatalWarnings" }

  filter "configurations:Debug"
    flags   { "Symbols" }
    optimize "Debug"
    defines { "DEBUG" }

  filter "configurations:Release"
    flags   { --[["LinkTimeOptimization"]] }
    optimize "Full"
    defines { "NDEBUG", "RELEASE" }

  filter {}

  -- CODE AND STUFF

  -- Externals
  for _, external in ipairs(os.matchfiles("scripts/externals/*.lua")) do
    include(external)
  end

  -- Packages
  local packages = os.matchfiles("source/*/package.lua")
  table.sort(packages)
  for _, pkg in ipairs(packages) do
    ian.loadPackage(string.explode(pkg, '/')[2], pkg)
  end
