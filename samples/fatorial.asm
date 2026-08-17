.name "programa"

.code
	sm20
	call F_main
	sys halt,0
F_fatorial:
	mov ax,1
	mov [V2],ax
L1:
	mov ax,0
	mov aw,[V1]
	push ax
	mov ax,1
	mov bx,ax
	pop ax
	cmp ax,bx
	jgt L2
	mov ax,0
	jmp L3
L2:
	mov ax,1
L3:
	cmp ax,0
	jz L4
	mov ax,[V2]
	push ax
	mov ax,0
	mov aw,[V1]
	mov bx,ax
	pop ax
	mul ax,bx
	mov [V2],ax
	mov ax,0
	mov aw,[V1]
	push ax
	mov ax,1
	mov bx,ax
	pop ax
	sub ax,bx
	mov [V1],aw
	jmp L1
L4:
	mov ax,[V2]
	ret
	ret
F_main:
	mov ax,S1
	push ax
	pop ax
	sys print,ax
	mov ax,1
	mov [V3],aw
L5:
	mov ax,0
	mov aw,[V3]
	push ax
	mov ax,13
	mov bx,ax
	pop ax
	cmp ax,bx
	jle L6
	mov ax,0
	jmp L7
L6:
	mov ax,1
L7:
	cmp ax,0
	jz L8
	mov ax,S2
	push ax
	mov ax,0
	mov aw,[V3]
	push ax
	pop bx
	pop ax
	sys prints,ax
	mov ax,bx
	sys printf,ax
	mov ax,S3
	push ax
	mov ax,0
	mov aw,[V3]
	push ax
	pop ax
	mov [V1],aw
	call F_fatorial
	push ax
	pop bx
	pop ax
	sys prints,ax
	mov ax,bx
	sys printf,ax
	mov ax,0
	mov aw,[V3]
	push ax
	mov ax,1
	mov bx,ax
	pop ax
	add ax,bx
	mov [V3],aw
	jmp L5
L8:
	mov ax,0
	push ax
	pop ax
	sys halt,ax
	ret

.data
V1:	dw 0
V2:	dd 0
V3:	dw 0
S1:	db "Calculando fatorial",10,0
S2:	db "Fatorial(%d)",0
S3:	db "=%d",10,0

.end
