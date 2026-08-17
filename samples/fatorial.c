/* Calcula e imprime o fatorial de 1 a 13, equivalente ao fatorial.asm */

long fatorial(int n) {
	long r;
	r = 1;
	while (n > 1) {
		r = r * n;
		n = n - 1;
	}
	return r;
}

void main() {
	int i;

	print("Calculando fatorial\n");
	i = 1;
	while (i <= 13) {
		printf("Fatorial(%d)", i);
		printf("=%d\n", fatorial(i));
		i = i + 1;
	}
	halt(0);
}
