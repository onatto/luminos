function clone_xform(xform, default_overrides)
    local clone = deepcopy(xform)
    -- This is to copy(clone) an adjusted(defaults=constant inputs changed)
    for input_name,default_value in pairs(default_overrides) do
        clone.inputs[input_name].default = default_value
    end
    return clone
end

  -- Cache the transform so we don't have to execute it again whenever it is called
  -- .cached is not the same with .visited
  -- This is also how you cache model loading transforms, just load once and the UI transform or any other transform that
  -- causes the change sets transform.cached = false, so if one kept the outputs from the transform too.
  --
  --
