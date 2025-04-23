all: asm c vm

../src/asm/asm.tab.c ../src/asm/asm.tab.h: ../src/asm/asm.y
	cd ../src/asm ; bison -d asm.y

../src/asm/lex.yy.c: ../src/asm/asm.l ../src/asm/asm.tab.h
	cd ../src/asm ; flex asm.l

../src/c/c.tab.c ../src/c/c.tab.h: ../src/c/c.y
	cd ../src/c ; bison -d c.y

../src/c/lex.yy.c: ../src/c/c.l ../src/c/c.tab.h
	cd ../src/c ; flex c.l

../run/asm.exe: ../src/asm/asm.tab.c ../src/asm/asm.tab.h ../src/asm/lex.yy.c ../src/include/arpia.h
	cd ../src/asm ; gcc -o ../../run/asm.exe asm.tab.c -ly -L /lib

../run/vm.exe: ../src/vm/vm.c ../src/include/arpia.h ../src/sys/system.h ../src/sys/system.c
	cd ../src/vm ; gcc -o ../../run/vm.exe vm.c

../run/c.exe: ../src/c/c.tab.c ../src/c/c.tab.h ../src/c/lex.yy.c ../src/include/arpia.h
	cd ../src/c ; gcc -o ../../run/c.exe c.tab.c -ly -L /lib

asm: ../run/asm.exe

vm: ../run/vm.exe
	
c: ../run/c.exe

clean:
	rm ../run/asm.exe ../src/asm/asm.tab.c ../src/asm/lex.yy.c ../src/asm/asm.tab.h ../run/c.exe ../src/c/c.tab.c ../src/c/lex.yy.c ../src/c/c.tab.h ../run/vm.exe	
	
