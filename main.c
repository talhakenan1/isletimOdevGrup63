/*
 * Talha Kenan Yaylacık B211210099 1C
 * Beyzanur Karaçam G211210054 2C
 * 
 * main.c
 * Linux Kabuk (Shell) Uygulaması ana dosyası
 */

#include "shell.h"

int main(void) {
    char line[MAX_LINE];
    Command cmd;
    
    // Shell'i başlat
    initialize_shell();
    handle_signals();
    
    printf("Linux Kabuk'a hoş geldiniz! Çıkmak için 'quit' yazın.\n");
    
    while (1) {
        // Prompt göster
        printf("> ");
        fflush(stdout);
        
        // Komutu oku
        read_command(line);
        
        // Boş komut kontrolü
        if (strlen(line) == 0) {
            continue;
        }
        
        // Quit komutu kontrolü
        if (strcmp(line, "quit") == 0) {
            if (bg_count > 0) {
                printf("Arkaplan işlemleri tamamlanana kadar bekleniyor...\n");
                while (bg_count > 0) {
                    sleep(1);
                }
            }
            printf("Kabuk kapatılıyor...\n");
            break;
        }
        
        // Çoklu komut kontrolü (noktalı virgül)
        if (strchr(line, ';') != NULL) {
            execute_multiple_commands(line);
            continue;
        }
        
        // Pipe kontrolü
        if (strchr(line, '|') != NULL) {
            execute_pipe_commands(line);
            continue;
        }
        
        // Tek komutu ayrıştır ve çalıştır
        parse_command(line, &cmd);
        if (cmd.arg_count > 0) {
            execute_command(&cmd);
        }
        cleanup_command(&cmd);
    }
    
    return 0;
}