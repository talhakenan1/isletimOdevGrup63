/*
 * Talha Kenan Yaylacık B211210099 1C
 * Beyzanur Karaçam G211210054 2C
 * 
 * shell.h
 * Linux Kabuk (Shell) Uygulaması için header dosyası
 */

#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>

// Sabitler
#define MAX_LINE 80        // Maksimum komut satırı uzunluğu
#define MAX_ARGS 40        // Maksimum argüman sayısı
#define MAX_BG_PROCESSES 100 // Maksimum arkaplan process sayısı
#define MAX_COMMANDS 10    // Noktalı virgülle ayrılmış maksimum komut sayısı

// Komut yapısı
typedef struct {
    char *args[MAX_ARGS];     // Komut argümanları
    char *input_file;         // Giriş yönlendirme dosyası
    char *output_file;        // Çıkış yönlendirme dosyası
    int background;           // Arkaplan işlem bayrağı
    int arg_count;            // Argüman sayısı
} Command;

// Arkaplan proseslerin takibi için yapı
typedef struct {
    pid_t pid;               // Process ID
    int active;              // Aktif/Pasif durumu
    char command[MAX_LINE];  // Çalıştırılan komut
} BackgroundProcess;

// Global değişkenler
extern BackgroundProcess bg_processes[MAX_BG_PROCESSES];
extern int bg_count;

// Fonksiyon prototipleri
void initialize_shell(void);
void read_command(char *line);
void parse_command(char *line, Command *cmd);
void execute_command(Command *cmd);
void handle_background_process(pid_t pid, const char *command);
void check_background_processes(void);
void cleanup_command(Command *cmd);
void execute_pipe_commands(char *line);
void execute_multiple_commands(char *line);
void handle_signals(void);

#endif /* SHELL_H */ 