local helpers = {}
function helpers.shallowCopy(orig)
  local t2 = {}
  for k,v in pairs(orig) do
    t2[k] = v
  end
  return t2
end

function helpers.SplitWhitespace(str)
    tokens = {}
    for token in str:gmatch("%S+") do 
        table.insert(tokens, token)
    end
    return tokens
end

function helpers.deepCopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[helpers.deepCopy(orig_key)] = helpers.deepCopy(orig_value)
        end
        setmetatable(copy, helpers.deepCopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

function helpers.tableLength(T)
  local count = 0
  for _ in pairs(T) do count = count + 1 end
  return count
end

function helpers.split(str, pat)
  local t = {}  -- NOTE: use {n = 0} in Lua-5.0
  local fpat = "(.-)" .. pat
  local last_end = 1
  local s, e, cap = str:find(fpat, 1)
  while s do
    if s ~= 1 or cap ~= "" then
      table.insert(t,cap)
    end
    last_end = e+1
    s, e, cap = str:find(fpat, last_end)
  end
  if last_end <= #str then
    cap = str:sub(last_end)
    table.insert(t, cap)
  end
  return t
end

return helpers
