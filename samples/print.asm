.name "print"

.code
m32
mov ax,1235
sys param, ax
sys param, ax
sys param, mascara
sys print,3
sys halt,0

.data
mascara:
db "%d (dec) = %x (hex)",13,10,0

.end
