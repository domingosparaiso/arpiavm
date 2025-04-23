.name "teste instrucoes"

.code
		m16
		sys param, inicio
		sys print, 1
		sys param, s_mov
		sys print, 1
		mov ax,1234
		cmp ax,1234
		jne erro
		call ok
		sys param, s_add
		sys print, 1
		mov ax,7324
		add ax,1249
		cmp ax,8573
		jne erro
		call ok
		sys param, s_sub
		sys print, 1
		mov ax,7324
		sub ax,1249
		cmp ax,6075
		jne erro
		call ok
		sys param, s_mul
		sys print, 1
		mov ax,732
		mul ax,10
		cmp ax,7320
		jne erro
		call ok
		sys param, s_div
		sys print, 1
		mov ax,7324
		div ax,100
		cmp ax,73
		jne erro
		cmp r7,24
		jne erro
		call ok
		sys param, s_and
		sys print, 1
		mov ax,5A93h
		and ax,0FF0h
		cmp ax,0A90h
		jne erro
		call ok
		sys param, s_or
		sys print, 1
		mov ax,5A43h
		or  ax,0F20h
		cmp ax,5F63h
		jne erro
		call ok
		sys param, s_xor
		sys print, 1
		mov ax,5A81h
		xor ax,0F88h
		cmp ax,5509h
		jne erro
		call ok
		sys param, s_shl
		sys print, 1
		mov ax,0AAAAh
		shl ax,1
		cmp ax,5554h
		jne erro
		call ok
		sys param, s_shr
		sys print, 1
		mov ax,5555h
		shr ax,1
		cmp ax,2AAAh
		jne erro
		call ok
		sys param, s_rol
		sys print, 1
		mov ax,0AAAAh
		rol ax,1
		cmp ax,05555h
		jne erro
		call ok
		sys param, s_ror
		sys print, 1
		mov ax,0AAAAh
		ror ax,1
		cmp ax,05555h
		jne erro
		call ok
		sys param, s_cmps
		sys print, 1
		cmps str1, str2
		jne erro
		call ok
		sys param, s_movs
		sys print, 1
		movs str1, str3
		cmps str2, str3
		jne erro
		call ok
		sys param, s_movnz
		sys print, 1
		mov cx,5
		movnz str1, str4
		cmps str0, str4
		jne erro
		call ok
		sys param, s_inc
		sys print, 1
		mov ax,6253
		inc ax
		cmp ax,6254
		jne erro
		call ok
		sys param, s_dec
		sys print, 1
		mov ax,5362
		dec ax
		cmp ax,5361
		jne erro
		call ok
		sys param, s_push
		sys print, 1
		mov ax,sp
		mov bx,259
		push bx
		sub ax,2
		cmp ax,sp
		jne erro
		call ok
		sys param, s_pop
		sys print, 1
		pop ax
		cmp ax,259
		jne erro
		call ok
		sys param, s_not
		sys print, 1
		mov ax,3741h
		not ax
		cmp ax,0C8BEh
		jne erro
		call ok
		sys param, s_loop
		sys print, 1
		mov cx,10
		mov dx,50
loop1:	dec dx
		loop loop1
		cmp dx,40
		jne erro
		call ok		
		sys param, s_loopnz
		sys print, 1
		mov ax,5
		mov dx,50
loop2:	inc dx
		dec ax
		loopnz loop2
		cmp dx,55
		jne erro
		call ok
		sys param, s_loopne
		sys print, 1
		mov ax,0
		mov dx,50
loop3:	inc dx
		inc ax
		cmp ax,4
		loopne loop3
		cmp dx,54
		jne erro
		call ok
		sys param, s_jgt
		sys print, 1
		cmp 2,1
		jgt l_jgt
		jmp erro
l_jgt:	call ok		
		sys param, s_jge
		sys print, 1
		cmp 2,2
		jge l_jge1
		jmp erro
l_jge1:	cmp 2,1
		jge l_jge2
		jmp erro
l_jge2: call ok
		sys param, s_jlt
		sys print, 1
		cmp 1,2
		jlt l_jlt
		jmp erro
l_jlt:	jeq erro
		call ok
		sys param, s_jle
		sys print, 1
		cmp 2,2
		jle l_jle1
		jmp erro
l_jle1:	cmp 1,2
		jle l_jle2
		jmp erro
l_jle2:	call ok
		sys param, s_jeq
		sys print, 1
		cmp 2,2
		jeq l_jeq
		jmp erro
l_jeq:	call ok
		sys param, s_jne
		sys print, 1
		cmp 2,1
		jne l_jne
		jmp erro
l_jne:	call ok
		sys param, s_jc
		sys print, 1
		mov ax,1
		shr ax,1
		jc l_jc
		jmp erro
l_jc:	call ok
		sys param, s_jnc
		sys print, 1
		mov ax,2
		shr ax,1
		jnc l_jnc
		jmp erro
l_jnc:	call ok		
		sys halt,0
ok:		sys param, msg_ok
		sys print, 1
		ret
erro:	sys param, msg_erro
		sys print, 1
		sys halt,1

.data
; DADOS		
str0:		db "TESTE",0
str1:		db "TESTE DE STRING 0",0
str2:		db "TESTE DE STRING 0",0
str3:		db "XXXXXXXXXXXXXXXXX",0
str4:		db "XXXXX",0
inicio:		db "Iniciando testes (16 bits)", 13, 10, 0
msg_erro:	db "...ERRO", 13, 10, 0
msg_ok:		db "...OK", 13, 10, 0
s_mov:		db "MOV",0
;s_cmp:		db "CMP",0
s_sys:		db "SYS",0
s_add:		db "ADD",0
s_sub:		db "SUB",0
s_mul:		db "MUL",0
s_div:		db "DIV",0
s_and:		db "AND",0
s_or:		db "OR",0
s_xor:		db "XOR",0
;s_in:		db "IN",0
;s_out:		db "OUT",0
s_shl:		db "SHL",0
s_shr:		db "SHR",0
s_rol:		db "ROL",0
s_ror:		db "ROR",0
s_cmps:		db "CMPS",0
s_movs:		db "MOVS",0
s_movnz:	db "MOVNZ",0
s_inc:		db "INC",0
s_dec:		db "DEC",0
s_push:		db "PUSH",0
s_pop:		db "POP",0
s_not:		db "NOT",0
s_loop:		db "LOOP",0
s_loopnz:	db "LOOPNZ",0
s_loopne:	db "LOOPNE",0
s_jgt:		db "JGT",0
s_jge:		db "JGE",0
s_jlt:		db "JLT",0
s_jle:		db "JLE",0
s_jeq:		db "JEQ",0
s_jne:		db "JNE",0
s_jc:		db "JC",0
s_jnc:		db "JNC",0
;s_jmp:		db "JMP",0
;s_call:	db "CALL",0
;s_ret:		db "RET",0
;s_nop:		db "NOP",0
s_m8:		db "M8",0
s_m16:		db "M16",0
s_m32:		db "M32",0

.end
