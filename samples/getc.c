void main() {
	int i;
	int c;
	outputpin(2);

	print("\n-- Pressione qualquer tecla --\n");
	output(2, 1);
	c=getch(0);
	output(2, 0);

	for(i=0; i<10; i=i+1) {
		delay(500);
		output(2,1);
		printf("[%d]...", 10-i);
		delay(500);
		output(2,0);
	}
	print("\nOK!\n\n");
}
