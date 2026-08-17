.name "teste instrucoes"

.code
		sm20
		sys print, inicio
		sys print, s_mov
		mov ax,1234
		cmp ax,1234
		jnz erro
		call ok
		sys print, s_add
		mov ax,7324
		add ax,1249
		cmp ax,8573
		jnz erro
		call ok
		sys print,s_sub
		
		mov ax,7324
		sub ax,1249
		cmp ax,6075
		jnz erro
		call ok
		sys print,s_mul
		
		mov ax,732
		mul ax,10
		cmp ax,7320
		jnz erro
		call ok
		sys print,s_div
		
		mov ax,7324
		div ax,100
		cmp ax,73
		jnz erro
		cmp dx,24
		jnz erro
		call ok
		sys print,s_and
		
		mov ax,5A93h
		and ax,0FF0h
		cmp ax,0A90h
		jnz erro
		call ok
		sys print,s_or
		
		mov ax,5A43h
		or  ax,0F20h
		cmp ax,5F63h
		jnz erro
		call ok
		sys print,s_xor
		
		mov ax,5A81h
		xor ax,0F88h
		cmp ax,5509h
		jnz erro
		call ok
		sys print,s_shl
		
		mov ax,0
		mov aw,0AAAAh
		shl aw,1
		cmp aw,5554h
		jnz erro
		call ok
		sys print,s_shr
		
		mov aw,5555h
		shr aw,1
		cmp aw,2AAAh
		jnz erro
		call ok
		sys print,s_rol
		
		mov aw,0AAAAh
		rol aw,1
		cmp aw,05555h
		jnz erro
		call ok
		sys print,s_ror
		
		mov aw,0AAAAh
		ror aw,1
		cmp aw,05555h
		jnz erro
		call ok
		sys print,s_cmpastr
		
		str
		cmpa str1, str2
		jnz erro
		call ok
		sys print,s_movastr
		
		str
		mova str3, str1
		cmpa str2, str3
		jnz erro
		call ok
		sys print,s_movabin
		
		bin
		mov cx,5
		mova str4, str1
		cmpa str0, str4
		jnz erro
		call ok
		sys print,s_inc
		
		mov ax,6253
		inc ax
		cmp ax,6254
		jnz erro
		call ok
		sys print,s_dec
		
		mov ax,5362
		dec ax
		cmp ax,5361
		jnz erro
		call ok
		sys print,s_push
		
		mov ax,sp
		mov bx,259
		push bx
		sub ax,4
		cmp ax,sp
		jnz erro
		call ok
		sys print,s_pop
		
		pop ax
		cmp ax,259
		jnz erro
		call ok
		sys print,s_not
		
		mov aw,3741h
		not aw
		cmp aw,0C8BEh
		jnz erro
		call ok
		sys print,s_loop
		
		mov cx,10
		mov dx,50
loop1:	dec dx
		loop loop1
		cmp dx,40
		jnz erro
		call ok		
		sys print,s_loopnz
		
		mov ax,5
		mov dx,50
loop2:	inc dx
		dec ax
		loopnz loop2
		cmp dx,55
		jnz erro
		call ok
		sys print,s_jgt
		
		cmp 2,1
		jgt l_jgt
		jmp erro
l_jgt:	call ok		
		sys print,s_jge
		
		cmp 2,2
		jge l_jge1
		jmp erro
l_jge1:	cmp 2,1
		jge l_jge2
		jmp erro
l_jge2: call ok
		sys print,s_jlt
		
		cmp 1,2
		jlt l_jlt
		jmp erro
l_jlt:	jz erro
		call ok
		sys print,s_jle
		
		cmp 2,2
		jle l_jle1
		jmp erro
l_jle1:	cmp 1,2
		jle l_jle2
		jmp erro
l_jle2:	call ok
		sys print,s_jz
		
		cmp 2,2
		jz l_jz
		jmp erro
l_jz:	call ok
		sys print,s_jnz
		
		cmp 2,1
		jnz l_jnz
		jmp erro
l_jnz:	call ok
		sys print,s_jc
		
		mov ax,1
		shr ax,1
		jc l_jc
		jmp erro
l_jc:	call ok
		sys print,s_jnc
		
		mov ax,2
		shr ax,1
		jnc l_jnc
		jmp erro
l_jnc:	call ok		
		sys halt,0
ok:		sys print,msg_ok
		
		ret
erro:	sys print,msg_erro
		
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
s_cmpastr:	db "CMPA",0
s_movastr:	db "MOVA(str)",0
s_movabin:	db "MOVA(bin)",0
s_inc:		db "INC",0
s_dec:		db "DEC",0
s_push:		db "PUSH",0
s_pop:		db "POP",0
s_not:		db "NOT",0
s_loop:		db "LOOP",0
s_loopnz:	db "LOOPNZ",0
s_jgt:		db "JGT",0
s_jge:		db "JGE",0
s_jlt:		db "JLT",0
s_jle:		db "JLE",0
s_jz:		db "JZ",0
s_jnz:		db "JNZ",0
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
