#ifndef VM_IOT_H
#define VM_IOT_H
#include <Arduino.h>
unsigned char *getmemory();
void reset_vm();
void decode();
void execute();
unsigned int shutdown_vm(unsigned int state);
void vmsyscall(unsigned int p1, unsigned int p2);
void run();
void list_dir();
void disk_format();
void file_exec(String filename);
void file_upload();
unsigned int getmemval(unsigned int pointer, int mode);
void setmemval(unsigned int pointer, unsigned int value, int mode);
unsigned int getregval(unsigned int nreg);
void setregval(unsigned int nreg, unsigned int value);

#endif