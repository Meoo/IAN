
ian = {}


-- ####################################
-- ###      OPTIONS
-- ####################################

newoption {
   trigger     = "ian-js",
   description = "Generate project for Javascript target with Emscripten",
}


-- ####################################
-- ###      GLOBALS
-- ####################################

IAN_JS    = _OPTIONS["ian-js"] ~= nil

if IAN_JS and (_ACTION and _ACTION ~= "clean" and _ACTION ~= "gmake") then
  premake.warn("'--ian-js' should be used with the action 'gmake'")
end

if IAN_JS then
  BIN_DIR = _WORKING_DIR.."/bin/em"
else
  BIN_DIR = _WORKING_DIR.."/bin"
end

OBJ_DIR   = BIN_DIR.."/obj"
MAKE_DIR  = BIN_DIR.."/make"


-- ####################################
-- ###      PACKAGES
-- ####################################

function ian.loadPackage(name, path)
  print("Loading package '".. name .."'")

  --local cwd = os.getcwd()

  -- local env = getfenv()
  -- setfenv(0, table)

  --local ok, err = pcall(include, path)
  include(path)

  -- setfenv(0, env)

  --os.chdir(cwd)

  --if not ok then
  --  error(err, 0)
  --end
end



-- ####################################
-- ###      SOLUTION
-- ####################################

function ian.solution(name)
  solution(name)
  configurations { "Release", "Debug" }

  -- DIRECTORIES
  location ( BIN_DIR )
  targetdir( BIN_DIR )
  objdir   ( OBJ_DIR )

  if IAN_JS then
    toolset("clang")

  else
    -- PLATFORMS
    if os.is64bit() then
      platforms { "x86_64", "x86" }
      defaultplatform "x86_64"
    else
      platforms { "x86", "x86_64" }
      defaultplatform "x86"
    end

    filter "platforms:x86"
      architecture "x86"
    filter "platforms:*x86_64"
      architecture "x86_64"
    filter {}
  end

end


-- ####################################
-- ###      PROJECTS
-- ####################################

function ian.external(name)
  project(name)
  location(MAKE_DIR)
  virtualproject(true)
end

function ian.project(name)
  project(name)
  location(MAKE_DIR)
end

function ian.module(name, mode)
  ian.project(name)

  -- Added to both client and server JS build
  local function jsTargetCommon()
    defines "IAN_JS"

    filter { "kind:SharedLib or *App" }
      targetextension ".js"
      flags "NoImportLib"

    filter {}
  end

  if mode == "server_c" then

    -- Server C code
    if IAN_JS then
      virtualproject(true)
    end
    kind "StaticLib"
    defines "IAN_SERVER"

  elseif mode == "server_js" then

    -- Server JS code
    if not IAN_JS then
      virtualproject(true)
    end
    targetdir(BIN_DIR .."/server")
    defines "IAN_SERVER"

    jsTargetCommon()

  elseif mode == "client_js" then

    -- Client JS code
    if not IAN_JS then
      virtualproject(true)
    end
    targetdir(BIN_DIR .."/client")
    defines "IAN_CLIENT"
    
    jsTargetCommon()

  else
    premake.warn("Invalid module mode : '"..tostring(mode).."'")
    virtualproject(true)
  end
end


-- ####################################
-- ###      EMSCRIPTEN HELPERS
-- ####################################

function ian.emOption(opt, value)
  if type(value) == "number" then
    return "-s ".. opt .."=".. value
  elseif type(value) == "table" then
    return "-s ".. opt .."=\"[".. string.gsub(table.implode(value, "'", "'", ","), "([\\\"])", "\\%1") .."]\""
  else
    return "-s ".. opt .."=\"".. string.gsub(tostring(value), "([\\\"])", "\\%1") .."\""
  end
end

function ian.emOptionFile(opt, path, relativeTo)
  return ian.emOption(opt, "@"..path.getabsolute(path, relativeTo))
end

function ian.emDefaultConfig()
  linkoptions {
    ian.emOption("NO_EXIT_RUNTIME", 1),
    ian.emOption("NO_FILESYSTEM", 1),
    ian.emOption("EXPORTED_RUNTIME_METHODS", {"ccall", "cwrap"}),
  }
end

function ian.emSafeConfig()
  ian.emDefaultConfig()
  linkoptions {
    ian.emOption("INVOKE_RUN", 0),
    ian.emOption("NO_DYNAMIC_EXECUTION", 1),
  }
end

function ian.emModularize(name)
  targetname(name)
  linkoptions {
    ian.emOption("EXPORT_NAME", "'".. name .."'"), -- TODO Is this a bug?
    ian.emOption("MODULARIZE", 1),
  }
end


-- ####################################
-- ###      SUBSCRIPTS
-- ####################################

include "ian/deploy.lua"

