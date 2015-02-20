xform concat
module core
dispname Concat

inputs
str str_left = Hello
str str_right =  World!
str seperator

outputs
str str_concat

func eval
out.str_concat = inp.str_left .. inp.seperator .. inp.str_right

func delete
debugger.print("Deleting concat " .. inp.str_left)
