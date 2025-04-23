.name "teste"
.map e1 "entrada"
.map s1 "saida"

.code
mov ax,135
mov dx,ax
call printdec
mov bx,238
mov dx,bx
call printdec
add ax,bx
mov dx,ax
call printdec
sys halt,0

printdec:
mov cx,dig
proximo:
div dx,10
add r7,30h
M8
mov (cx),r7
M16
dec cx
cmp dx,0
jne proximo
prox:
cmp cx,msg
jlt printok
M8
mov (cx),20h
M16
dec cx
jmp prox
printok:
sys param, msg
sys print, 1
ret

.data
msg:
db "0000"
dig:
db "0",13,10,0

.end
