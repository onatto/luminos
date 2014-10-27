local xform = {}
local debugger = require "debugger"

local function shallowcopy(orig)
  local t2 = {}
  for k,v in pairs(orig) do
    t2[k] = v
  end
  return t2
end

local function deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

function xform.clone(transform, default_overrides)
    if transform.clone ~= nil then
        return transform.clone(transform, default_overrides)
    end
    local clone = deepcopy(transform)
    -- This is to copy(clone) an adjusted(defaults=constant inputs changed)
    for input_name,default_value in pairs(default_overrides) do
        clone.inputs[input_name].default = default_value
    end
    return clone
end

return xform

  -- Cache the transform so we don't have to execute it again whenever it is called
  -- .cached is not the same with .visited
  -- This is also how you cache model loading transforms, just load once and the UI transform or any other transform that
  -- causes the change sets transform.cached = false, so if one kept the outputs from the transform too.
  --
  --
