.name "indef"

.code
sys param, msg
sys print, 1
sys halt,0

.data
msg:
db "123",0
db ?, ?
db 1
db dup(10) ?

.end
