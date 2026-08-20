#define IOT
#include <Arduino.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include "arpia.h"
#include "vm-arpia.h"

#include <Arduino.h>
#include <LittleFS.h>
#include "vm-iot.h"
#define VERSION "1.02"

#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
WebServer webserver(80);
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer webserver(80);
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "changeme"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "changeme"
#endif
#define WIFI_AP_SSID "ArpiaVM"
#define WIFI_AP_PASSWORD "arpiavm123"
#define WIFI_CONNECT_TIMEOUT_MS 15000
#include "syscall-iot.h"

File uploadFile;

int debug_term = 0;
String vmIP = "";
String VM_WIFI_SSID = WIFI_SSID;
String VM_WIFI_PASSWORD = WIFI_PASSWORD;
String VM_WIFI_AP_SSID = WIFI_AP_SSID;
String VM_WIFI_AP_PASSWORD = WIFI_AP_PASSWORD;

String read_input() {
    String result = "";
    int ch = 0;
    while(ch != 10 && ch != 13) {
        webserver.handleClient();
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
  reset_vm();
	while(shutdown_vm(GETSTATUS)!=TRUE) {
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
//    Serial.printf("File Size: %d bytes\n", fileSize);

    size_t bytesRead = readFile.read(getmemory(), fileSize);
//    Serial.printf("Successfully read %d bytes.\n", bytesRead);
    run();
    Serial.println("Program terminated.");
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
    size_t bytesRead = readFile.read(getmemory(), fileSize);
    getmemory()[fileSize] = 0;
    Serial.printf("%s\n", getmemory());
}

#define MAX_HEX_DUMP_LINE 32
void file_dump(String filename) {
    File readFile = LittleFS.open("/" + filename, "r");
    if (!readFile) {
        Serial.printf("Failed to open file %s for reading!\n", filename);
        return;
    }
    size_t fileSize = readFile.size();
    size_t bytesRead = readFile.read(getmemory(), fileSize);
    size_t i = 0;
    while(i<fileSize) {
        Serial.print("| ");
        for(int d=i; d<i+MAX_HEX_DUMP_LINE; d++) {
            if(d<fileSize) {
                Serial.printf("%02X ", *(getmemory()+d));
            } else {
                Serial.print("   ");
            }
        }
        Serial.print("| ");
        for(int d=i; d<i+MAX_HEX_DUMP_LINE; d++) {
            if(d<fileSize) {
                char c = *(getmemory()+d);
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

String strip_leading_slash(String name) {
    if (name.length() > 0 && name[0] == '/') name = name.substring(1);
    return name;
}

String html_escape(String s) {
    s.replace("&", "&amp;");
    s.replace("\"", "&quot;");
    s.replace("<", "&lt;");
    s.replace(">", "&gt;");
    return s;
}

void load_wifi_credentials(int showmsg) {
    File credFile = LittleFS.open("/wifi.txt", "r");
    if (!credFile) return;
    String line = credFile.readStringUntil('\n');
    line.trim();
    int sepIndex = line.indexOf(',');
    if (sepIndex != -1) {
        VM_WIFI_SSID = line.substring(0, sepIndex);
        VM_WIFI_PASSWORD = line.substring(sepIndex + 1);
        VM_WIFI_SSID.trim();
        VM_WIFI_PASSWORD.trim();
        if(showmsg) Serial.printf("Loaded WiFi credentials from wifi.txt: SSID=\"%s\"\n", VM_WIFI_SSID.c_str());
    } else {
        if(showmsg) Serial.println("Invalid format in wifi.txt. Expected format: SSID,PASSWORD");
    }
    line = credFile.readStringUntil('\n');
    line.trim();
    sepIndex = line.indexOf(',');
    if (sepIndex != -1) {
        VM_WIFI_AP_SSID = line.substring(0, sepIndex);
        VM_WIFI_AP_PASSWORD = line.substring(sepIndex + 1);
        VM_WIFI_AP_SSID.trim();
        VM_WIFI_AP_PASSWORD.trim();
    }
}

void wifi_edit_credentials() {
    Serial.printf("\nEnter WiFi SSID [%s]: ", VM_WIFI_SSID);
    String vm_ssid = read_input();
    Serial.printf("\nEnter WiFi Password [%s]: ", VM_WIFI_PASSWORD);
    String vm_password = read_input();
    if (vm_ssid.length() > 0) VM_WIFI_SSID = vm_ssid;
    if (vm_password.length() > 0) VM_WIFI_PASSWORD = vm_password;
    Serial.printf("\nFailover when no Wifi found.\nEnter AP SSID [%s]: ", VM_WIFI_AP_SSID);
    vm_ssid = read_input();
    Serial.printf("\nEnter AP Password [%s]: ", VM_WIFI_AP_PASSWORD);
    vm_password = read_input();
    if (vm_ssid.length() > 0) VM_WIFI_AP_SSID = vm_ssid;
    if (vm_password.length() > 0) VM_WIFI_AP_PASSWORD = vm_password;
}

void wifi_save_credentials(int showmsg) {
    File credFile = LittleFS.open("/wifi.txt", FILE_WRITE);
    if (!credFile) {
        if(showmsg) Serial.println("\nFail to access wifi.txt file\n");
        return;
    }
    credFile.printf("%s,%s\n", VM_WIFI_SSID, VM_WIFI_PASSWORD);
    credFile.printf("%s,%s\n", VM_WIFI_AP_SSID, VM_WIFI_AP_PASSWORD);
    credFile.close();
    if(showmsg) Serial.println("\nWiFi credentials saved to wifi.txt\n");
    return;
}

void wifi_setup() {
    load_wifi_credentials(1);
    Serial.printf("Connecting to WiFi \"%s\" ", VM_WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(VM_WIFI_SSID, VM_WIFI_PASSWORD);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
        Serial.print(".");
    }
    Serial.println("");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi connected. IP address: ");
        Serial.println(WiFi.localIP());
        vmIP = WiFi.localIP().toString();
    } else {
        Serial.println("WiFi connection failed, starting Access Point...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP(VM_WIFI_AP_SSID, VM_WIFI_AP_PASSWORD);
        Serial.printf("Access Point \"%s\" started. IP address: ", VM_WIFI_AP_SSID);
        Serial.println(WiFi.softAPIP());
        vmIP = WiFi.softAPIP().toString();
    }
}

void web_handle_root() {
    String html;
    html.reserve(2048);
    html += "<!DOCTYPE html><html><head><meta charset='utf-8'>"
            "<title>Arpia VM - Arquivos</title>"
            "<style>body{font-family:sans-serif;margin:2em;}"
            "table{border-collapse:collapse;width:100%;max-width:640px;}"
            "th,td{border:1px solid #ccc;padding:0.4em 0.8em;text-align:left;}"
            "th{background:#eee;}form{margin-top:1.5em;}"
            "button{cursor:pointer;}</style></head><body>"
            "<h1>Arpia VM &mdash; LittleFS</h1>"
            "<table><tr><th>Arquivo</th><th>Tamanho</th><th></th></tr>";

    int filecount = 0;
    #ifdef ESP32
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory()) {
                String name = strip_leading_slash(String(file.name()));
                html += "<tr><td>" + name + "</td><td>" + String(file.size()) + " bytes</td>"
                        "<td><a href='/delete?file=" + name + "' "
                        "onclick=\"return confirm('Excluir " + name + "?');\">excluir</a></td></tr>";
                filecount++;
            }
            file = root.openNextFile();
        }
    #endif
    #ifdef ESP8266
        Dir dir = LittleFS.openDir("/");
        while (dir.next()) {
            if (dir.isFile()) {
                String name = strip_leading_slash(dir.fileName());
                html += "<tr><td>" + name + "</td><td>" + String(dir.fileSize()) + " bytes</td>"
                        "<td><a href='/delete?file=" + name + "' "
                        "onclick=\"return confirm('Excluir " + name + "?');\">excluir</a></td></tr>";
                filecount++;
            }
        }
    #endif
    if (filecount == 0) {
        html += "<tr><td colspan='3'>Nenhum arquivo encontrado.</td></tr>";
    }
    html += "</table>"
            "<h2>Upload de arquivo</h2>"
            "<form method='POST' action='/upload' enctype='multipart/form-data'>"
            "<input type='file' name='upload'> <button type='submit'>Enviar</button>"
            "</form>"
            "<p><a href='/wifi'>Configurar WiFi</a></p>"
            "</body></html>";

    webserver.send(200, "text/html", html);
}

void web_handle_wifi_get() {
    load_wifi_credentials(0);

    String html;
    html.reserve(1536);
    html += "<!DOCTYPE html><html><head><meta charset='utf-8'>"
            "<title>Arpia VM - WiFi</title>"
            "<style>body{font-family:sans-serif;margin:2em;}"
            "label{display:block;margin-top:0.8em;}"
            "input{padding:0.3em;width:260px;max-width:100%;}"
            "button{cursor:pointer;margin-top:1.5em;}</style></head><body>"
            "<h1>Arpia VM &mdash; WiFi</h1>"
            "<form method='POST' action='/wifi'>"
            "<label>SSID<input type='text' name='ssid' value='" + html_escape(VM_WIFI_SSID) + "'></label>"
            "<label>Senha<input type='password' name='password' value='" + html_escape(VM_WIFI_PASSWORD) + "'></label>"
            "<label>SSID do Access Point (fallback)<input type='text' name='ap_ssid' value='" + html_escape(VM_WIFI_AP_SSID) + "'></label>"
            "<label>Senha do Access Point<input type='password' name='ap_password' value='" + html_escape(VM_WIFI_AP_PASSWORD) + "'></label>"
            "<button type='submit'>Salvar</button>"
            "</form>"
            "<p><a href='/'>&larr; Voltar</a></p>"
            "</body></html>";

    webserver.send(200, "text/html", html);
}

void web_handle_wifi_post() {
    if (webserver.hasArg("ssid")) VM_WIFI_SSID = webserver.arg("ssid");
    if (webserver.hasArg("password")) VM_WIFI_PASSWORD = webserver.arg("password");
    if (webserver.hasArg("ap_ssid")) VM_WIFI_AP_SSID = webserver.arg("ap_ssid");
    if (webserver.hasArg("ap_password")) VM_WIFI_AP_PASSWORD = webserver.arg("ap_password");
    wifi_save_credentials(0);
    webserver.sendHeader("Location", "/wifi", true);
    webserver.send(303);
}

void web_handle_delete() {
    if (webserver.hasArg("file")) {
        file_delete(strip_leading_slash(webserver.arg("file")));
    }
    webserver.sendHeader("Location", "/", true);
    webserver.send(303);
}

void web_handle_upload_data() {
    HTTPUpload& upload = webserver.upload();
    String filename = "/" + strip_leading_slash(upload.filename);
    if (upload.status == UPLOAD_FILE_START) {
//        Serial.printf("Web upload start: %s\n", filename.c_str());
        uploadFile = LittleFS.open(filename, "w");
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) {
            uploadFile.close();
 //           Serial.printf("Web upload done: %s, %u bytes\n", filename.c_str(), upload.totalSize);
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        if (uploadFile) uploadFile.close();
//        Serial.println("Web upload aborted");
    }
}

void web_handle_upload_done() {
    webserver.sendHeader("Location", "/", true);
    webserver.send(303);
}

void web_server_setup() {
    webserver.on("/", HTTP_GET, web_handle_root);
    webserver.on("/wifi", HTTP_GET, web_handle_wifi_get);
    webserver.on("/wifi", HTTP_POST, web_handle_wifi_post);
    webserver.on("/delete", HTTP_GET, web_handle_delete);
    webserver.on("/upload", HTTP_POST, web_handle_upload_done, web_handle_upload_data);
    webserver.onNotFound([]() { webserver.send(404, "text/plain", "Not found"); });
    webserver.begin();
    Serial.println("Web server started.");
}

void setup() {
    Serial.begin(115200);
    Serial.printf("\n\n"
                "Arpia VM %s\n", VERSION);
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Disk Mount Failed!");
        return;
    }
    Serial.println("LittleFS Disk Mounted Successfully.");
    wifi_setup();
    web_server_setup();
    Serial.println("Minishell started.\nSystem ready!\n");
    Serial.print("# ");
}

#define TOKEN_NONE    0
#define TOKEN_DIR     1
#define TOKEN_VERSION 2
#define TOKEN_HELP    3
#define TOKEN_FORMAT  4
#define TOKEN_DELETE  5
#define TOKEN_CAT     6
#define TOKEN_DUMP    7
#define TOKEN_CLEAR   8
#define TOKEN_TERM    9
#define TOKEN_WIFI    10
#define TOKEN_REBOOT  11

void loop() {
    int command_token = TOKEN_NONE;
    String param = "";
    String commands[] = {"ls", "dir", "ver", "version", "help", "?", "format", "del", "rm", "cat", "type", "dump", "cls", "clear", "term", "wifi", "reboot"};
    const int command_tokens[] = { TOKEN_DIR, TOKEN_DIR, TOKEN_VERSION, TOKEN_VERSION, TOKEN_HELP, TOKEN_HELP, TOKEN_FORMAT, TOKEN_DELETE, TOKEN_DELETE, TOKEN_CAT, TOKEN_CAT, TOKEN_DUMP, TOKEN_CLEAR, TOKEN_CLEAR, TOKEN_TERM, TOKEN_WIFI, TOKEN_REBOOT };
    String command = read_input();
    Serial.println("");
    if(command != "") {
        int endIndex = command.indexOf(' ');
        if(endIndex != -1) {
            String token = command.substring(0, endIndex);
            param = command.substring(endIndex + 1);
            command = token;
        }
        for(int c=0; c<17; c++) {
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
                Serial.print("[HELP]\n"
                                "ls/dir........ list disk contents\n"
                                "ver/version... show version\n"
                                "format........ format disk\n"
                                "rm/del........ delete file\n"
                                "cat/type ..... show file contents\n"
                                "dump ......... dump hex file\n"
                                "cls/clear .... clear screen\n"
                                "reboot ....... restart device\n"
                                "wifi ......... configure wifi SSID/Password\n"
                                "<filename>.... execute file\n"
                                "web UI........ http://");
                Serial.print(vmIP);
                Serial.println("/ (list/upload/delete files)\n\n");
                break;
            case TOKEN_FORMAT:
                disk_format();
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
            case TOKEN_WIFI:
                load_wifi_credentials(1);
                wifi_edit_credentials();
                wifi_save_credentials(1);
                break;
            case TOKEN_REBOOT:
                ESP.restart();
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