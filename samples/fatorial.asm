.name "fatorial"

.code
	sys print, instrucoes
	m32
	mov cx,13
	mov ax,1
	mov dx,1
	call printfat
	sys param, modo16
	sys print, 1
	m16
	mov cx,13
	mov ax,1
	mov dx,1
	call printfat
	
	sys halt, 0
printfat:	
	mul dx,ax
	sys param, ax
	sys param, masc1
	sys print, 2
	inc ax
	sys param, dx
	sys param, masc2
	sys print, 2
	loop printfat
	ret
	

.data
instrucoes:	db "Mostrando a diferenca entre os modos 16 e 32 bits",13,10,"Modo=32 bits",13,10,0
modo16:		db "Modo=16 bits",13,10,0
masc1:		db "Fatorial(%d)",0
masc2:		db "=%d",13,10,0
.end
