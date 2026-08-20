#ifndef VM_X86_H
#define VM_X86_H
void displaydebug(void);
void vmsyscall(unsigned int p1, unsigned int p2);
void print_str(const char *s);
void print_num(const char *fmt, ...);
void a_print(const char *s);
void a_printd(const char *fmt, ...);
void debug(unsigned char *msg);
void run();
const char * nomeinstrucao();
int loadram(char *nomearquivo);
void displayend(int e, int p, int v);
void dumpmem();
void displaydebug();
void set_debugmode(int mode);
int get_debugmode();
#endif