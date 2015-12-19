
require "export"
require "virtualproject"

mw = {}


BIN_DIR   = _WORKING_DIR.."/bin"
OBJ_DIR   = _WORKING_DIR.."/bin/obj"
MAKE_DIR  = _WORKING_DIR.."/bin/make"


-- ####################################
-- ###      OPTIONS
-- ####################################

local _, options = pcall(dofile, _MAIN_SCRIPT_DIR.."/user-options.lua")
if type(options) ~= "table" then
  options = {}
end

for k, v in pairs(_OPTIONS) do
  options[k] = v
end

_OPTIONS = options

function mw.booloption(option, default)
  local value = _OPTIONS[option]
  if not value then return default end
  if value:lower() == "true" then return true end
  if value:lower() == "false" then return false end
  premake.error(value.." is not a valid boolean value", 1)
end


-- ####################################
-- ###      EXTERNALS
-- ####################################

function mw.external(name)
  virtualproject(name)
    location(MAKE_DIR)
end


-- ####################################
-- ###      MODULES
-- ####################################

function mw.module(name)
  project(name)
    location(MAKE_DIR)
end


-- ####################################
-- ###      PROJECTS
-- ####################################

function mw.project(name)
  project(name)
    location(MAKE_DIR)
end

-- ####################################
-- ###      UTILS
-- ####################################

function mw.processfile(input, output, translations)

  local function translate(s)
    local t = translations[s]
    if not t then
      premake.warn("No translation for $"..s.."$ in file "..input)
      return tostring(s)
    end
    if type(t) == "function" then
      return tostring(t(s))
    end
    return tostring(translations[s])
  end

  local fin = io.open(input, "r")
  local fout = io.open(output, "w+")

  for line in fin:lines() do
    fout:write(line:gsub("%$([A-Z_]-)%$", translate).."\n")
  end

  fin:close()
  fout:close()
end

function mw.concattextfiles(filesArray)
  local tmp = os.tmpname()
  local fout = io.open(tmp, "w+")

  for _, input in ipairs(filesArray) do
    local fin = io.open(input, "r")
    for line in fin:lines() do
      fout:write(line.."\n")
    end
    fin:close()
  end

  fout:close()
  return tmp
end
