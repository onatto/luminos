module core

in str str_left = Hello
in str str_right =  World!

out str str_concat

xform Concat
out.str_concat = inp.str_left .. inp.str_right

return function ()
    debugger.print("Deleting concat with " .. inp.str_left)
end
