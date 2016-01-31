
ian.deploy = {}
local id = ian.deploy


-- ####################################
-- ###      OPTIONS
-- ####################################

id.directory = _MAIN_SCRIPT_DIR .."/deploy"

if _ACTION == "deploy-dev" then
  id.release = false
  id.dev = true
else
  id.release = true
  id.dev = false
end

do
  local ok, options = pcall(dofile, _MAIN_SCRIPT_DIR.."/deploy-options.lua")

  id.options = ok and options or {}

  if not ok and (_ACTION == "deploy" or _ACTION == "deploy-dev") then
    premake.warn("Failed to load deploy-options.lua, using default values")
  end
end

function id.getOption(key, default)
  local keys = string.explode(key, "%.")
  local lastkey = table.remove(keys)

  local t = id.options
  for _, v in ipairs(keys) do
    if t[v] == nil then
      return default
    end
    t = t[v]
    if type(t) ~= "table" then
      premake.warn("When accessing option '".. key .."', '".. v .."' is not a table")
      return default
    end
  end

  if id.dev and t[lastkey ..":dev"] ~= nil then
    return t[lastkey ..":dev"]
  end
  if t[lastkey] ~= nil then
    return t[lastkey]
  end
  return default
end


-- ####################################
-- ###      HELPERS
-- ####################################

local function filesString(count)
  return count == 1 and "1 file" or count .." files"
end

local deployList = {}

function id.addDeployCommand(tab, level)
  if type(tab) == "function" then
    tab = { deploy = tab }
  end

  local lv = (level or 0) + 2
  local info = debug.getinfo(lv, "Sl")
  tab._file = info.source
  tab._line = info.currentline
  table.insert(deployList, tab)
end

local function doDeploy(release)
  local pwd = os.getcwd()
  os.rmdir(id.directory)
  os.mkdir(id.directory)
  os.chdir(id.directory)

  for _, v in ipairs(deployList) do
    local ok, err = pcall(v.deploy, v)
    if not ok then
      if v.filename then
        print("Deploy failed for target '".. v.filename .."'")
        error(err, 0)
      else
        print("Deploy failed for declaration ".. v._file ..":".. v._line)
        error(err, 0)
      end
    end
  end

  os.chdir(pwd)
end


-- ####################################
-- ###      THIRD PARTY TOOLS
-- ####################################

local THIRDPARTY_DIR = _MAIN_SCRIPT_DIR.."/third-party"
local YUI_VERSION = "2.4.8"

local function runClosureCompiler(target, files)
  if not os.isfile(THIRDPARTY_DIR .."/closure/compiler.jar") then
    print("Downloading Closure compiler")
    local tmp = os.tmpname()
    http.download("http://dl.google.com/closure-compiler/compiler-latest.zip", tmp)
    zip.extract(tmp, THIRDPARTY_DIR .."/closure")
    os.remove(tmp)
  end

  --local tmp = mw.concattextfiles(GAME_CL)
  local ok = os.execute("java -jar ".. THIRDPARTY_DIR .."/closure/compiler.jar --js_output_file ".. target .." --js ".. table.concat(files, " "))
  --os.remove(tmp)
  if ok ~= 0 then
    error("JS compilation failed", 0)
  end
end

local function runYuiCompressor(target, files)
  if not os.isfile(THIRDPARTY_DIR .."/yui/yuicompressor-"..YUI_VERSION..".jar") then
    print("Downloading YUI compressor")
    local tmp = os.tmpname()
    http.download("http://github.com/yui/yuicompressor/releases/download/v"..YUI_VERSION.."/yuicompressor-"..YUI_VERSION..".zip", tmp)
    zip.extract(tmp, THIRDPARTY_DIR .."/yui")
    os.remove(tmp)
  end

  --local tmp = mw.concattextfiles(GAME_CL_CSS)
  local ok = os.execute("java -jar ".. THIRDPARTY_DIR .."/yui/yuicompressor-"..YUI_VERSION..".jar -o ".. target .." --type css ".. table.concat(files, " "))
  --os.remove(tmp)
  if ok ~= 0 then
    error("CSS compilation failed", 0)
  end
end


-- ####################################
-- ###      JS TARGETS
-- ####################################

local compiledJsMeta = {
  
  addFiles = function(self, ...)
    local files = table.flatten({...})
    for k, v in ipairs(files) do
      if not path.isabsolute(v) then
        files[k] = path.getabsolute(v)
      end
    end
    self.sources = table.join(self.sources, files)
  end,

  expandSources = function(self)
    local finalSources = {}
    for _, src in ipairs(self.sources) do
      if string.find(src, "*") == nil then
        table.insert(finalSources, src)
      else
        for _, match in ipairs(os.matchfiles(src)) do
          table.insert(finalSources, match)
        end
      end
    end
    self.sources = finalSources
  end,

  deploy = function(self, release)
    self:expandSources()

    if id.release then
      -- Release
      print("Compiling JS '".. self.filename .."' from ".. filesString(#self.sources))

      local p = path.getdirectory(self.filename)
      if not os.isdir(p) then
        os.mkdir(p)
      end

      runClosureCompiler(self.filename, self.sources)

    else
      -- Development
      local f = io.open(self.filename, "w+")
      f:write("ian_dev.load([".. table.implode(self.sources, "\"file://", "\"", ",\n") .."]);")
      f:close()
    end
  end,

}
compiledJsMeta.__index = compiledJsMeta

function id.compiledJs(filename)
  local t = {
    filename = filename,
    sources = {},
  }
  setmetatable(t, compiledJsMeta)
  id.addDeployCommand(t, 1)
  return t
end


-- ####################################
-- ###      CSS TARGETS
-- ####################################

local compiledCssMeta = {
  
  addFiles = function(self, ...)
    local files = table.flatten({...})
    for k, v in ipairs(files) do
      if not path.isabsolute(v) then
        files[k] = path.getabsolute(v)
      end
    end
    self.sources = table.join(self.sources, files)
  end,

  expandSources = function(self)
    local finalSources = {}
    for _, src in ipairs(self.sources) do
      if string.find(src, "*") == nil then
        table.insert(finalSources, src)
      else
        for _, match in ipairs(os.matchfiles(src)) do
          table.insert(finalSources, match)
        end
      end
    end
    self.sources = finalSources
  end,

  deploy = function(self, release)
    self:expandSources()

    if id.release then
      -- Release
      print("Compiling CSS '".. self.filename .."' from ".. filesString(#self.sources))
      
      local p = path.getdirectory(self.filename)
      if not os.isdir(p) then
        os.mkdir(p)
      end

      runYuiCompressor(self.filename, self.sources)

    else
      -- Development
      local f = io.open(self.filename, "w+")
      for _, v in ipairs(self.sources) do
        f:write("@import url(\"file://".. v .."\");\n")
      end
      f:close()
    end
  end,

}
compiledCssMeta.__index = compiledCssMeta

function id.compiledCss(filename)
  local t = {
    filename = filename,
    sources = {},
  }
  setmetatable(t, compiledCssMeta)
  id.addDeployCommand(t, 1)
  return t
end


-- ####################################
-- ###      TEXT TARGETS
-- ####################################

local mergedTextMeta = {

  addFiles = function(self, ...)
    local files = table.flatten({...})
    for k, v in ipairs(files) do
      if not path.isabsolute(v) then
        files[k] = path.getabsolute(v)
      end
    end
    self.sources = table.join(self.sources, files)
  end,

  expandSources = function(self)
    local finalSources = {}
    for _, src in ipairs(self.sources) do
      if string.find(src, "*") == nil then
        table.insert(finalSources, src)
      else
        for _, match in ipairs(os.matchfiles(src)) do
          table.insert(finalSources, match)
        end
      end
    end
    self.sources = finalSources
  end,

  deploy = function(self, release)
    self:expandSources()
    print("Building '".. self.filename .."' from ".. filesString(#self.sources))

    local p = path.getdirectory(self.filename)
    if not os.isdir(p) then
      os.mkdir(p)
    end

    local tmp = mw.concattextfiles(self.sources)
    if not os.rename(tmp, self.filename) then
      error("Failed to move merged text file to destination '"..self.filename.."'", 0)
    end
  end

}
mergedTextMeta.__index = mergedTextMeta

function id.mergedTextFiles(filename)
  local t = {
    filename = filename,
    sources = {},
  }
  setmetatable(t, mergedTextMeta)
  id.addDeployCommand(t, 1)
  return t
end


local transformedTextMeta = {
  
  deploy = function(self, release)
    mw.processfile(self.source, self.filename, self.transform)
  end

}
transformedTextMeta.__index = transformedTextMeta

function id.transformedTextFile(filename, source, transform)
  local t = {
    filename = filename,
    source = path.isabsolute(source) and source or path.getabsolute(source),
    transform = transform or {},
  }
  setmetatable(t, transformedTextMeta)
  id.addDeployCommand(t, 1)
  return t
end


local generatedTextMeta = {
  
  deploy = function(self, release)
    local p = path.getdirectory(self.filename)
    if not os.isdir(p) then
      os.mkdir(p)
    end

    local f = io.open(self.filename, "w+")
    self.generator(f)
    f:close()
  end

}
generatedTextMeta.__index = generatedTextMeta

function id.generatedText(filename, generator)
  local t = {
    filename = filename,
    generator = generator,
  }
  setmetatable(t, generatedTextMeta)
  id.addDeployCommand(t, 1)
  return t
end


-- ####################################
-- ###      ACTIONS
-- ####################################

newaction {
  trigger = "deploy",
  description = "Package scripts and binaries for release",

  execute = function()
    doDeploy(false)
  end,
}

newaction {
  trigger = "deploy-dev",
  description = "Package scripts and binaries for development",

  execute = function()
    doDeploy(true)
  end,
}
