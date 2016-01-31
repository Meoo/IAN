
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






















local success, DEPLOY_OPTIONS = pcall(dofile, _MAIN_SCRIPT_DIR.."/deploy-options.lua")

if not success and (_ACTION == "deploy" or _ACTION == "deploy-dev") then
  premake.warn("No deploy-options.lua, using default values")
end

local HTML_TITLE  = DEPLOY_OPTIONS.title or "IAN Client"
local HTML_META   = DEPLOY_OPTIONS.meta or {}

local CORNER_LINKS = DEPLOY_OPTIONS.links or {}

local SERVER_ADDR = DEPLOY_OPTIONS["server-addr"] or "localhost"
local SERVER_PORT = DEPLOY_OPTIONS["server-port"] or "7011"

local MAGIC = DEPLOY_OPTIONS["magic"] or "49616e49616e49616e49616e49616e21"

local TLS = DEPLOY_OPTIONS.tls or {}
local TLS_CERT = TLS.cert
local TLS_CERTCHAIN = TLS.certchain
local TLS_KEY = TLS.key
local TLS_KEYPASS = TLS.keypass
local TLS_DH = TLS.dh

local JQUERY_VERSION = "2.1.4"
local YUI_VERSION = "2.4.8"
local FONTAWESOME_VERSION = "4.5.0"

local GAME_COMPILED = false

local GAME_RC     = {}
local GAME_RC_CNT = 0
local GAME_CL     = {}
local GAME_CL_CSS = {}
local GAME_SV     = {}

local HTML_TRANSLATE = {
  TITLE = HTML_TITLE,
  JQUERY_VERSION = JQUERY_VERSION,
  FONTAWESOME_VERSION = FONTAWESOME_VERSION,
  SERVER_ADDR = SERVER_ADDR,
  SERVER_PORT = SERVER_PORT,
  GAME_MAGIC = MAGIC,
  META = function()
    local s = ""
    for k, v in pairs(HTML_META) do
      s = s .."<meta name=\"".. k .."\" content=\"".. v .."\">"
    end
    return s
  end,
  CSS = function()
    local s = ""
    for _, v in ipairs(GAME_CL_CSS) do
      if GAME_COMPILED then
        s = s .."<link rel=\"stylesheet\" type=\"text/css\" href=\"".. v .."\">"
      else
        s = s .."<link rel=\"stylesheet\" type=\"text/css\" href=\"file://".. path.getabsolute(v) .."\">"
      end
    end
    return s
  end,
  JS = function()
    local s = ""
    for _, v in ipairs(GAME_CL) do
      if GAME_COMPILED then
        s = s .."<script type=\"text/javascript\" src=\"".. v .."\"></script>"
      else
        s = s .."<script type=\"text/javascript\" src=\"file://".. path.getabsolute(v) .."\"></script>"
      end
    end
    return s
  end,
  CORNER_LINKS = function()
    local s = ""
    for _, v in ipairs(CORNER_LINKS) do
      s = s .."<a href=\""..v[3].."\"><i class=\"fa fa-"..v[2].." fa-2x\"></i>"..v[1].."</a>"
    end
    return s
  end,
}

-- ///////////////////////////////////////////////////// --

local function filesString(count)
  return count == 1 and "1 file" or count .." files"
end


local function prepareFolders()
    os.rmdir("deploy/client")
    os.rmdir("deploy/server")

    os.mkdir("deploy/client")
    os.mkdir("deploy/server")
    os.mkdir("third-party")
end


local function downloadUtils()
  if not os.isfile("third-party/closure/compiler.jar") then
    print("Downloading Closure compiler")
    local tmp = os.tmpname()
    http.download("http://dl.google.com/closure-compiler/compiler-latest.zip", tmp)
    zip.extract(tmp, "third-party/closure")
  end

  if not os.isfile("third-party/yui/yuicompressor-"..YUI_VERSION..".jar") then
    print("Downloading YUI compressor")
    local tmp = os.tmpname()
    http.download("http://github.com/yui/yuicompressor/releases/download/v"..YUI_VERSION.."/yuicompressor-"..YUI_VERSION..".zip", tmp)
    zip.extract(tmp, "third-party/yui")
  end
end


local function findGameFiles()
  local ignored_files = 0
  local skip_files = { "game/client.html" }

  for _, filename in ipairs(os.matchfiles("game/**")) do
    if not table.contains(skip_files, filename) then
      local res = filename:match("/rc%.(.*)$")
      if res then
        -- Folder or file starts with rc.
        -- File is a resource
        if GAME_RC[res] then
          premake.warn("Resource ".. res .." match multiple files")
        else
          GAME_RC[res] = filename
          GAME_RC_CNT = GAME_RC_CNT + 1
        end

      else
        -- Folder or file starts with /cl. or /0.cl.
        local cl = filename:find("/cl%.") or filename:find("/%d+%.cl%.")
        -- Folder or file starts with /sv. or /0.sv.
        local sv = filename:find("/sv%.") or filename:find("/%d+%.sv%.")

        if cl and sv then
          premake.warn("Both cl. and sv. found in path : ".. filename)
        end

        if cl and (not sv or cl < sv) then
          -- If /cl. only or if /cl. appears before /sv.
          -- Client file
          local ext = path.getextension(filename)
          if ext == ".js" then
            table.insert(GAME_CL, filename)
          elseif ext == ".css" then
            table.insert(GAME_CL_CSS, filename)
          else
            ignored_files = ignored_files + 1
          end

        elseif sv and (not cl or sv < cl) then
          -- If /sv. only or if /sv. appears before /cl.
          -- Server file
          local ext = path.getextension(filename)
          if ext == ".js" then
            table.insert(GAME_SV, filename)
          elseif ext == ".css" then
            premake.warn("Found CSS in server files : ".. filename)
            ignored_files = ignored_files + 1
          else
            ignored_files = ignored_files + 1
          end

        else
          -- Shared file
          local ext = path.getextension(filename)
          if ext == ".js" then
            table.insert(GAME_SV, filename)
            table.insert(GAME_CL, filename)
          elseif ext == ".css" then
            table.insert(GAME_CL_CSS, filename)
          else
            ignored_files = ignored_files + 1
          end

        end
      end
    end
  end

  if ignored_files > 0 then
    premake.warn("Ignored ".. filesString(ignored_files) .." in game folder")
  end
end


local function sortGameFiles()
  local function sortFunc(a, b)
    local at = string.explode(a, "/")
    local bt = string.explode(b, "/")

    -- Find first different folder / name
    -- Ignore "game", start at second folder
    local i = 1
    local ap, bp
    repeat
      i = i + 1
      ap = at[i]
      bp = bt[i]
    until not ap or ap ~= bp

    -- Sometimes a == b... thanks lua
    if not ap then
      return false
    end

    -- Files before folders
    if #at == i then
      if #bt > i then
        return true
      end
    elseif #bt == i then
      return false
    end

    -- User order
    local ao = ap:match("^(%d+)%.")
    local bo = bp:match("^(%d+)%.")

    if ao then
      if bo then
        if ao == bo then
          -- Same order, sort alphabetically
          return ap:match("^%d+%.(.*)$") < bp:match("^%d+%.(.*)$")
        else
          -- Sort by order
          return tonumber(ao) < tonumber(bo)
        end
      else
        return true
      end
    elseif bo then
      return false
    end

    -- Alphabetically
    return ap < bp
  end

  table.sort(GAME_SV, sortFunc)
  table.sort(GAME_CL, sortFunc)
  table.sort(GAME_CL_CSS, sortFunc)
end


local function compileGameFiles()
  downloadUtils()
  local s

  -- Client CSS
  print("Compiling client CSS (".. filesString(#GAME_CL_CSS) ..")")
  local tmp = mw.concattextfiles(GAME_CL_CSS)
  s = os.execute("java -jar third-party/yui/yuicompressor-"..YUI_VERSION..".jar -o deploy/client/ian.css --type css ".. tmp)
  if s ~= 0 then
    premake.error("Could not compile client CSS")
  end
  os.remove(tmp)

  -- Client JS
  print("Compiling client JS (".. filesString(#GAME_CL) ..")")
  --local tmp = mw.concattextfiles(GAME_CL)
  s = os.execute("java -jar third-party/closure/compiler.jar --js_output_file deploy/client/ian.js --js ".. table.concat(GAME_CL, " "))
  if s ~= 0 then
    premake.error("Could not compile client JS")
  end
  os.remove(tmp)

  -- Server JS
  print("Compiling server JS (".. filesString(#GAME_SV) ..")")
  --local tmp = mw.concattextfiles(GAME_SV)
  s = os.execute("java -jar third-party/closure/compiler.jar --js_output_file deploy/server/ian.js --js ".. table.concat(GAME_SV, " "))
  if s ~= 0 then
    premake.error("Could not compile server JS")
  end
  os.remove(tmp)

  GAME_CL_CSS = { "ian.css" }
  GAME_CL     = { "ian.js" }
  GAME_COMPILED = true
end


local function copyResources()
  print("Copying client resources (".. filesString(GAME_RC_CNT) ..")")
  for dest, src in pairs(GAME_RC) do
    os.mkdir("deploy/client/"..path.getdirectory(dest))
    local ok, err = os.copyfile(src, "deploy/client/"..dest)
    if not ok then
      premake.warn("Resource copy failed for ".. src .." : ".. err)
    end
  end
end


local function genServerConfig()
  if not GAME_COMPILED then
    local f = io.open("deploy/server/js.manifest", "w+")
    for _, v in ipairs(GAME_SV) do
      f:write("../../".. v .."\n")
    end
    f:close()
  end

  local f = io.open("deploy/server/server.cfg", "w+")
  f:write("debug="..tostring(not GAME_COMPILED).."\n")
  f:write("magic="..MAGIC.."\n")
  if TLS_CERT then f:write("tls.cert="..TLS_CERT.."\n") end
  if TLS_CERTCHAIN then f:write("tls.certchain="..TLS_CERTCHAIN.."\n") end
  if TLS_KEY then f:write("tls.key="..TLS_KEY.."\n") end
  if TLS_KEYPASS then f:write("tls.pass="..TLS_KEYPASS.."\n") end
  if TLS_DH then f:write("tls.dh="..TLS_DH.."\n") end
  f:close()
end

-- ///////////////////////////////////////////////////// --

newaction {
  trigger = "olddeploy",
  description = "Package client and server for release",

  execute = function()
    prepareFolders()

    findGameFiles()
    sortGameFiles()
    compileGameFiles()

    mw.processfile("game/client.html", "deploy/client/index.html", HTML_TRANSLATE)
    copyResources()

    genServerConfig()

    print("TODO Server")
  end,
}

-- ///////////////////////////////////////////////////// --

newaction {
  trigger = "olddeploy-dev",
  description = "Package client and server for development",

  execute = function()
    prepareFolders()

    findGameFiles()
    sortGameFiles()

    mw.processfile("game/client.html", "deploy/client/index.html", HTML_TRANSLATE)
    copyResources()

    genServerConfig()

    print("TODO Server")
  end,
}
