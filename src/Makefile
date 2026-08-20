all:	asm c vm

./asm/asm.tab.c ./asm/asm.tab.h: ./asm/asm.y
	cd ./asm ; bison -d asm.y

./asm/lex.yy.c: ./asm/asm.l ./asm/asm.tab.h
	cd ./asm ; flex asm.l

./c/c.tab.c ./c/c.tab.h: ./c/c.y
	cd ./c ; bison -d c.y

./c/lex.yy.c: ./c/c.l ./c/c.tab.h
	cd ./c ; flex c.l

../bin/asm.exe: ./asm/asm.tab.c ./asm/asm.tab.h ./asm/lex.yy.c ./vm-iot/arpia.h
	cd ./asm ; gcc -o ../../bin/asm.exe asm.tab.c -L /lib

../bin/c.exe: ./c/c.tab.c ./c/c.tab.h ./c/lex.yy.c ./vm-iot/arpia.h
	cd ./c ; gcc -o ../../bin/c.exe c.tab.c -L /lib

../bin/vm.exe: ./vm/src/vm-x86.c ./vm/include/arpia.h ./vm/include/vm-arpia.h ./vm/include/syscall-x86.h ./vm/include/vm-x86.h ./vm/include/getch-x86.h
	cd ./vm/src ; gcc -o ../../../bin/vm.exe vm-x86.c

asm: ../bin/asm.exe

c: ../bin/c.exe

vm: ../bin/vm.exe

clean:
	rm ../bin/asm.exe ./asm/asm.tab.c ./asm/lex.yy.c ./asm/asm.tab.h
	rm ../bin/c.exe ./c/c.tab.c ./c/lex.yy.c ./c/c.tab.h
	rm ../bin/vm.exe
