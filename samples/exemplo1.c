#description "Interruptor Iluminação Simples"
#define BOTAO "Botao (IN)"
#define RELE "Rele (OUT)"

int estado;

int setup() {
	estado = input(BOTAO);
	output(RELE, estado);
	ontimer(1,function);
}

int function() {
	int valor;

	valor = input(BOTAO);
	if(valor != estado) {
		print("(Botao)");
		output(RELE, valor);
		estado = valor;
	}
}
