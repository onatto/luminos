local util = {}
local ui = require 'ui'
local core = require 'core'
local debugger = require 'debugger'

-- So, this is a module inside a 'lua module'
-- Basically a namespacing way, nothing more
-- Same thing could be done with local inport = { ... } and
-- returning exports_array = {inport, print_xform, ..., }
-- Oh, well, saving a lot of, I'm in util namespace lest I forget, since
-- I have to type in THE namespace(by typing variable name) when I require it locally too
-- Well, what are the consequences of then
--
-- The consequence is that this code is compiled to a function returns multiple variables
util.inport = {
    name = "Util Port",
    is_hub = true,
    inputs = {},
    outputs = {},
    eval = function(self)
    end
}

util.print_xform = {
    name = "Print",
    inputs = {
        str = {type = "string", default=""},
        y = {type = "number", default=6}
    },
    outputs = {
        this = {type = "this", value="This transform prints" }
    },
    eval = function(self)
        ui.dbgText(self.inputs.y.value, tostring(self.inputs.str.value))
    end
}

return util
