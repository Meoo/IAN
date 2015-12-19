
require "scripts/mwb"


solution "IAN"
  language "C++"
  configurations { "Release", "Debug" }

  -- DIRECTORIES
  targetdir( BIN_DIR )
  objdir   ( OBJ_DIR )
  
  -- PLATFORMS
  if os.is64bit() then
    platforms { "native64", "native32" }
    defaultplatform "native64"
  else
    platforms { "native32", "native64" }
    defaultplatform "native32"
  end

  filter "platforms:*32"
    architecture "x86"
  filter "platforms:*64"
    architecture "x86_64"
  filter {}

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

  -- Projects
  include "src"

  -- Other actions
  include "scripts/deploy"
