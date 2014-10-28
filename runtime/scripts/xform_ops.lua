local xform = {}
local helpers = require "helpers"

function xform.clone(transform, default_overrides)
    if transform.clone ~= nil then
        return transform.clone(transform, default_overrides)
    end
    local clone = helpers.deepCopy(transform)
    -- This is to copy(clone) an adjusted(defaults=constant inputs changed)
    for input_name,default_value in pairs(default_overrides) do
        clone.inputs[input_name].default = default_value
    end
    return clone
end

return xform
