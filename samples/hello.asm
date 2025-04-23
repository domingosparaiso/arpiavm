.name "hello"

.code
m32
sys param, msg
sys print,1
sys halt,0

.data
msg:
db "Hello, world!",13,10,0

.end
