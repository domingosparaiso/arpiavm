#define IOT 1
#include <Arduino.h>
#include <LittleFS.h>
#include "vm-iot.h"
#include "vm-arpia.h"
#include "syscall.h"
#define VERSION "1.00"

int debug_term = 0;

String read_input() {
    String result = "";
    int ch = 0;
    while(ch != 10 && ch != 13) {
        if (Serial.available() > 0) {
            ch = Serial.read();
            if(debug_term) Serial.printf("[%02X]", ch);
            if(ch>=32 && ch <= 126) {
                Serial.print((char) ch);
                result += String((char) ch);
            } else {
                switch(ch) {
                    case 3: // CTRL+C
                        Serial.print("<CTRL+C>\n# ");
                        result = "";
                        break;
                    case 127: // BACKSPACE
                        if(result.length() > 0) {
                            Serial.print("\x7F \x7F");
                            result.remove(result.length() - 1);
                        }
                        break;
                }
            }
        }
    }
    result.trim();
    return(result);
}

void run() {
	reg.ip = 0;
	reg.sp = MAXRAM;
	haltsystem = FALSE;
	while(!haltsystem) {
		decode();
		execute();
	}
}

void print_file(String filename, long int filesize) {
    const int max_len = 40;
    filename += " ";
    while (filename.length() < max_len) filename += ".";
    Serial.print(filename);
    Serial.printf(" %d\n", filesize);
}

void list_dir() {
    Serial.println("Listing directory...");
    int filecount = 0;
    String fileheader = "FILENAME ............................... FILESIZE\n";
    #ifdef ESP32
        File root = LittleFS.open("/");
        if(!root){
            Serial.println("- failed to open directory");
            return;
        }
        if(!root.isDirectory()){
            Serial.println(" - not a directory");
            return;
        }

        File file = root.openNextFile();
        if(file) Serial.print(fileheader);
        while(file){
            if(!file.isDirectory()){
                print_file(String(file.name()), file.size());
                filecount++;
            }
            file = root.openNextFile();
        }
    #endif

    #ifdef ESP8266
        Dir dir = LittleFS.openDir("/");

        if(dir.next()) Serial.print(fileheader);
        while(dir.next()) {
            if (dir.isFile()) {
                // Print file metadata
                print_file(String(dir.fileName()), dir.fileSize());
                filecount++;
            }
        }    
    #endif
    if(filecount > 0) {
        Serial.printf("%d file(s).\n", filecount);
    } else {
        Serial.println("No files found.");
    }
}

void disk_format() {
    Serial.println("LittleFS disk formating...");
    LittleFS.format();
    Serial.println("\nFormat, done!");
}

void file_exec(String filename) {
    File readFile = LittleFS.open("/" + filename, "rb");
    if (!readFile) {
        Serial.printf("Failed to open file %s for reading!\n", filename);
        return;
    }

    size_t fileSize = readFile.size();
    Serial.printf("File Size: %d bytes\n", fileSize);

    size_t bytesRead = readFile.read(memory, fileSize);
    Serial.printf("Successfully read %d bytes.\n", bytesRead);
    run();
    Serial.println("Program terminated.");
}

void file_upload(String filename) {
	int d = 0;
	int state = 0;
	unsigned int fcrc = 0;
	unsigned int ccrc = 0;
	unsigned int i;
    unsigned char c = 0;

	maxmem = 0;
	i = 0;
    String line;

    Serial.print("filename: ");
    Serial.print(filename);
    Serial.print("\ncode: ");
    while(c != 10 && c != 13) {
        if(Serial.available() > 0) {
            c = (unsigned char) Serial.read();
            if(c != 10 && c != 13) {
                if(maxmem <= MAXRAM) {
                    if(c != ' ') {
                        c = c-'0'; if(c > 9) c -= 7; if(c > 15) c -= 32;
                        if(d == 0) {
                            d = 1;
                            i = c;
                        } else {
                            d = 0;
                            i = (i << 4) + c;
                            ccrc = (ccrc + i) & 0xFFFF;
                            memory[maxmem++]=(unsigned char)(i & 0xff);
                        }
                        c = 0;
                    }
                }
            }
        }
    }
    Serial.print("\ncrc: ");
    c = 0;
    String scrc = read_input();
    if(scrc.length() >= 4) {
        for(int i=0; i<4; i++) {
            c = scrc[i];
            if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                c=c-'0'; if(c>9) c-=7; if(c>15) c-=32;
                fcrc=(fcrc<<4)+c;
                c = 0;
            }
        }
    }
    Serial.println("\nOK\n");
	if(ccrc == fcrc) {
        Serial.print("File loaded into memory.\n");
		Serial.printf("CRC: OK\n");
        Serial.printf("Bytes: %d\n", maxmem);
        filename = "/" + filename;
        File file = LittleFS.open(filename, "wb");
        if (!file) {
            Serial.print("Failed to open file for writing\n");
            return;
        }
        size_t bytesWritten = file.write((const uint8_t*)&memory, maxmem);
        file.close();
        Serial.printf("Successfully wrote %d bytes.\n", bytesWritten);
	} else {
		Serial.printf("CRC Fail: Read=%04X Calc=%04X\n", fcrc, ccrc);
	}
}

void file_delete(String filename) {
    filename = "/" + filename;
    if(LittleFS.exists(filename)) {
        if(LittleFS.remove(filename)) {
            Serial.println("OK");
        } else {
            Serial.println("Delete fail");
        }
    } else {
        Serial.println("File not found.");
    }
}

void file_cat(String filename) {
    File readFile = LittleFS.open("/" + filename, "r");
    if (!readFile) {
        Serial.printf("Failed to open file %s for reading!\n", filename);
        return;
    }
    size_t fileSize = readFile.size();
    size_t bytesRead = readFile.read(memory, fileSize);
    memory[fileSize] = 0;
    Serial.printf("%s\n", memory);
}

#define MAX_HEX_DUMP_LINE 32
void file_dump(String filename) {
    File readFile = LittleFS.open("/" + filename, "r");
    if (!readFile) {
        Serial.printf("Failed to open file %s for reading!\n", filename);
        return;
    }
    size_t fileSize = readFile.size();
    size_t bytesRead = readFile.read(memory, fileSize);
    size_t i = 0;
    while(i<fileSize) {
        Serial.print("| ");
        for(int d=i; d<i+MAX_HEX_DUMP_LINE; d++) {
            if(d<fileSize) {
                Serial.printf("%02X ", memory[d]);
            } else {
                Serial.print("   ");
            }
        }
        Serial.print("| ");
        for(int d=i; d<i+MAX_HEX_DUMP_LINE; d++) {
            if(d<fileSize) {
                char c = memory[d];
                if(c < 32 || c > 126) c = '.';
                Serial.printf("%c",c);
            } else {
                Serial.print(" ");
            }
        }
        i+=MAX_HEX_DUMP_LINE;
        Serial.println(" |");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("\n\n"
                "-------------\n"
                "Arpia VM %s\n", VERSION);
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Disk Mount Failed!");
        return;
    }
    Serial.println("LittleFS Disk Mounted Successfully.");
    Serial.println("Minishell started.\nSystem ready!\n");
    Serial.print("# ");
}

#define TOKEN_NONE    0
#define TOKEN_DIR     1
#define TOKEN_VERSION 2
#define TOKEN_HELP    3
#define TOKEN_FORMAT  4
#define TOKEN_UPLOAD  5
#define TOKEN_DELETE  6
#define TOKEN_CAT     7
#define TOKEN_DUMP    8
#define TOKEN_CLEAR   9
#define TOKEN_TERM    10

void loop() {
    int command_token = TOKEN_NONE;
    String param = "";
    String commands[] = {"ls", "dir", "ver", "version", "help", "?", "format", "upload", "del", "rm", "cat", "type", "dump", "cls", "clear", "term"};
    const int command_tokens[] = { TOKEN_DIR, TOKEN_DIR, TOKEN_VERSION, TOKEN_VERSION, TOKEN_HELP, TOKEN_HELP, TOKEN_FORMAT, TOKEN_UPLOAD, TOKEN_DELETE, TOKEN_DELETE, TOKEN_CAT, TOKEN_CAT, TOKEN_DUMP, TOKEN_CLEAR, TOKEN_CLEAR, TOKEN_TERM };
    String command = read_input();
    Serial.println("");
    if(command != "") {
        int endIndex = command.indexOf(' ');
        if(endIndex != -1) {
            String token = command.substring(0, endIndex);
            param = command.substring(endIndex + 1);
            command = token;
        }
        for(int c=0; c<16; c++) {
            if (commands[c] == command) {
                command_token = command_tokens[c];
                break;
            }
        }
        switch(command_token) {
            case TOKEN_DIR:
                list_dir();
                break;
            case TOKEN_VERSION:
                Serial.printf("Arpia VM %s\nMinishell\n", VERSION);
                break;
            case TOKEN_HELP:
                Serial.println("[HELP]\n"
                                "ls/dir........ list disk contents\n"
                                "ver/version... show version\n"
                                "format........ format disk\n"
                                "upload........ upload file\n"
                                "rm/del........ delete file\n"
                                "cat/type ..... show file contents\n"
                                "dump ......... dump hex file\n"
                                "cls/clear .... clear screen\n"
                                "<filename>.... execute file\n\n");
                break;
            case TOKEN_FORMAT:
                disk_format();
                break;
            case TOKEN_UPLOAD:
                file_upload(param);
                break;
            case TOKEN_DELETE:
                file_delete(param);
                break;
            case TOKEN_CAT:
                file_cat(param);
                break;
            case TOKEN_DUMP:
                file_dump(param);
                break;
            case TOKEN_CLEAR:
                Serial.print("\x1B[2J\x1B[H");
                break;
            case TOKEN_TERM:
                debug_term = (debug_term==0)?1:0;
                break;
            default:
                if(command != "") {
                    file_exec(command);
                }
                break;
        }
    }
    Serial.print("# ");
}