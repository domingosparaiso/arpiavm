/* Demonstra struct, array e passagem de array/struct por referencia para
   funcoes (sempre por endereco, conforme a linguagem simplificada). */

struct Point {
	int x;
	int y;
};

void movepoint(struct Point p, int dx, int dy) {
	p.x = p.x + dx;
	p.y = p.y + dy;
}

void soma_array(int arr[], int n, int resultado[]) {
	int i;
	int total;
	total = 0;
	i = 0;
	while (i < n) {
		total = total + arr[i];
		i = i + 1;
	}
	resultado[0] = total;
}

struct Point origem;
int numeros[5];

void main() {
	int soma[1];

	origem.x = 10;
	origem.y = 20;
	movepoint(origem, 5, -3);
	printf("ponto.x=%d\n", origem.x);
	printf("ponto.y=%d\n", origem.y);

	numeros[0] = 1;
	numeros[1] = 2;
	numeros[2] = 3;
	numeros[3] = 4;
	numeros[4] = 5;
	soma_array(numeros, 5, soma);
	printf("soma=%d\n", soma[0]);

	halt(0);
}
