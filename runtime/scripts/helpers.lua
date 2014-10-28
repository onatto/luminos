local helpers = {}
function helpers.shallowCopy(orig)
  local t2 = {}
  for k,v in pairs(orig) do
    t2[k] = v
  end
  return t2
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

return helpers
