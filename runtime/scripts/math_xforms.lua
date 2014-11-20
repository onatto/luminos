local math_xforms = {}
math_xforms.sine_xform = {
    name = "Sin",
    -- Computes sin(rad * period + xoffset) * amplitude + yoffset
    inputs = {
        rad = {type = "number", default=0},
        period = {type = "number", default=1},
        -- Just transform this code to, number rad = 0, number period = 0 and you got
        -- that representation, what the hell, that was soo stupid... :D
        -- This is actually what a compiler should provide to a function.

        amplitude = {type = "number", default=1},
        xoffset = {type = "number", default=0},
        yoffset = {type = "number", default=0},
    },
    outputs = {
        sin = {type = "number"}
    },
    eval = function(self)
        self.outputs.sin.value = math.sin(self.inputs.rad.value * self.inputs.period.value + self.inputs.xoffset.value) * self.inputs.amplitude.value + self.inputs.yoffset.value
    end
}

return math_xforms
