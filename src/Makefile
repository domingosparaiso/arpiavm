all:
	asm
#	c
	vm

../src/asm/asm.tab.c ../src/asm/asm.tab.h: ../src/asm/asm.y
	cd ../src/asm ; bison -d asm.y

../src/asm/lex.yy.c: ../src/asm/asm.l ../src/asm/asm.tab.h
	cd ../src/asm ; flex asm.l

#../src/c/c.tab.c ../src/c/c.tab.h: ../src/c/c.y
#	cd ../src/c ; bison -d c.y

#../src/c/lex.yy.c: ../src/c/c.l ../src/c/c.tab.h
#	cd ../src/c ; flex c.l

../bin/asm.exe: ../src/asm/asm.tab.c ../src/asm/asm.tab.h ../src/asm/lex.yy.c ../src/include/arpia.h
	cd ../src/asm ; gcc -o ../../bin/asm.exe asm.tab.c -ly -L /lib

../bin/vm.exe: ../src/vm/vm.c ../src/include/arpia.h ../src/sys/system.h ../src/sys/system.c
	cd ../src/vm ; gcc -o ../../bin/vm.exe vm.c

#../bin/c.exe: ../src/c/c.tab.c ../src/c/c.tab.h ../src/c/lex.yy.c ../src/include/arpia.h
#	cd ../src/c ; gcc -o ../../bin/c.exe c.tab.c -ly -L /lib

asm: ../bin/asm.exe

vm: ../bin/vm.exe

#c: ../bin/c.exe

clean:
	rm ../bin/asm.exe ../src/asm/asm.tab.c ../src/asm/lex.yy.c ../src/asm/asm.tab.h
#	rm ../bin/c.exe ../src/c/c.tab.c ../src/c/lex.yy.c ../src/c/c.tab.h
	rm ../bin/vm.exe
