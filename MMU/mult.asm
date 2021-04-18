# Multiply two numbers, found in logical locations 12 and 13, leaving the result in location 14.
__main: 0
mov 0 14
mov *12 reg2
jez reg2 11
mov *13 reg1
jez reg1 11
mov reg2 reg0
sub 1 reg1
jez reg1 10
add reg0 reg2
jmp 6
mov reg2 14
end
__data: 3
