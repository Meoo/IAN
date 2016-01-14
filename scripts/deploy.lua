
local _, DEPLOY_OPTIONS = pcall(dofile, _MAIN_SCRIPT_DIR.."/deploy-options.lua")

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
  trigger = "deploy",
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
  trigger = "deploy-dev",
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
