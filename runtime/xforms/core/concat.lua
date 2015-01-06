module core

in str str_left = "hello"
in str str_right = "goodbye"

out str str_concat

xform Concat
out.str_concat = inp.str_left .. inp.str_right
