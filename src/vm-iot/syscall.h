#ifndef SYSCALL_H
#define SYSCALL_H
#include "arpia.h"
#include "vm-iot.h"

unsigned char *printfmask;
void vmsyscall(unsigned int p1, unsigned int p2) {
	switch(p1) {
		case SYS_PRINT:
			Serial.print(String((const char*) memory+p2));
			break;
		case SYS_PRINTS:
			printfmask = memory+p2;
			break;
		case SYS_PRINTF:
			{
				char *fmt = (char *) printfmask;
				char *p = strchr(fmt, '%');
				if(p) {
					p++;
					while(*p && strchr("-+ #0123456789.", *p)) p++;
					if(*p=='f' || *p=='g' || *p=='e') {
						union F conv;
						conv.i = p2;
						Serial.printf(fmt, (double)conv.f);
					} else if(*p=='s') {
						Serial.printf(fmt, (const char*)memory+p2);
					} else {
						Serial.printf(fmt, p2);
					}
				} else {
					Serial.printf(fmt);
				}
			}
			break;
		case SYS_DEBUG:
			break;
		case SYS_HALT:
			haltsystem = TRUE;
			break;
		case SYS_INPUTPIN:
			pinMode(p2, INPUT);
			break;
		case SYS_OUTPUTPIN:
			pinMode(p2, OUTPUT);
			break;
		case SYS_PULLUP:
			pinMode(p2, INPUT_PULLUP);
			break;			
		case SYS_GETCHAR:
			unsigned long start_time;
			int getch_timeout = FALSE;
			if(p2 > 0) start_time = millis();
			while(Serial.available() == 0 && !getch_timeout) {
				if(p2 > 0) {
					if(millis() - start_time > p2) {
						setregval(AX, 0);
						getch_timeout = TRUE;
					}
				}
			}
			if(!getch_timeout) {
				unsigned int ch = Serial.read();
				setregval(AX, ch);
			}
			break;
	}
}
#endif
