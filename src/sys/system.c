void syscall(unsigned int p1, unsigned int p2) {
	switch(p1) {
		case SYS_PRINT:
			printf(memory+p2);
			break;
		case SYS_PRINTS:
			printfmask = memory+p2;
			break;
		case SYS_PRINTF:
			printf(printfmask, p2);
			break;
		case SYS_DEBUG:
			if(debugmode) printf(memory+p2);
			break;
		case SYS_HALT:
			if(debugmode) printf("HALT SYSTEM\n");
			haltsystem = TRUE;
			break;
	}
}

