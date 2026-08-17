.name "hello"

.code
sm32
sys print, msg
sys halt,0

.data
msg:
db "Hello, world!",13,10,0

.end
