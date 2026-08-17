#ifndef SYSCALL_H
#define SYSCALL_H
#include <conio.h>
#include "vm.h"

unsigned char *printfmask;

void syscall(unsigned int p1, unsigned int p2) {
	char *fmt;
	char *p;
	union { unsigned int i; float f; } conv;

	switch(p1) {
		case SYS_PRINT:
			printf((char*)memory+p2);
			break;
		case SYS_PRINTS:
			printfmask = memory+p2;
			break;
		case SYS_PRINTF:
			fmt = (char*)printfmask;
			p = strchr(fmt, '%');
			if(p) {
				p++;
				while(*p && strchr("-+ #0123456789.", *p)) p++;
				switch(*p) {
					case 'f':
					case 'g':
					case 'e':
						conv.i = p2;
						printf(fmt, (double)conv.f);
						break;
					case 's':
						printf(fmt, (char*)memory+p2);
						break;
					default:
						printf(fmt, p2);
						break;
				}
			} else {
				printf(fmt);
			}
			break;
		case SYS_DEBUG:
			if(get_debugmode()) printf((char*)memory+p2);
			break;
		case SYS_HALT:
			if(get_debugmode()) printf("HALT SYSTEM\n");
			haltsystem = TRUE;
			break;
        case SYS_GETCHAR:
            setregval(AX, getch());
            break;
        case SYS_INPUTPIN:
            setregval(AX, 0);
            break;
        case SYS_OUTPUTPIN:
            setregval(AX, 0);
            break;
        case SYS_PULLUP:
            setregval(AX, 0);
            break;
	}
}
#endif