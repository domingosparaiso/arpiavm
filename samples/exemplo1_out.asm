.name "Interruptor Iluminacao Simples"
.map BOTAO "Botao (IN)"
.map RELE "Rele (OUT)"

.code
setup:
	in estado, BOTAO
	out RELE, estado
	sys timer, param_001
	sys halt, 0

function:
	in valor, BOTAO
	cmp [valor], [estado]
	jne label_001
	sys param, str_001
	sys print, 1
	out RELE, valor
	mov [estado], [valor]
label_001:	
	sys halt, 0

.data	
; dados inicializados
param_001:	dw  1
            dw  function	
str_001:
            db  "(botao)"
; dados não inicializados
estado:	dw 0
valor:	dw 0

.end
