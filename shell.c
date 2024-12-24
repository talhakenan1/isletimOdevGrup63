/*
 * Talha Kenan Yaylacık B211210099 1C
 * Beyzanur Karaçam G211210054 2C
 * 
 * shell.c
 * Linux Kabuk (Shell) Uygulaması implementasyon dosyası
 */

#include "shell.h"

// Global değişkenlerin tanımlanması
BackgroundProcess bg_processes[MAX_BG_PROCESSES];
int bg_count = 0;

// SIGCHLD sinyali için handler
void sigchld_handler(int signo) {
    pid_t pid;
    int status;
    
    // Zombie process'leri temizle
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_BG_PROCESSES; i++) {
            if (bg_processes[i].pid == pid && bg_processes[i].active) {
                printf("\n[%d] retval: %d\n", 
                       pid, WEXITSTATUS(status));
                printf("> "); // Prompt'u tekrar göster
                fflush(stdout);
                bg_processes[i].active = 0;
                bg_count--;
                break;
            }
        }
    }
}

// SIGINT (Ctrl+C) sinyali için handler
void sigint_handler(int signo) {
    printf("\n> ");
    fflush(stdout);
}

// Sinyal yönetimi
void handle_signals(void) {
    struct sigaction sa_chld, sa_int;
    
    // SIGCHLD için handler ayarla
    sa_chld.sa_handler = sigchld_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa_chld, NULL);
    
    // SIGINT için handler ayarla
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);
}

void execute_command(Command *cmd) {
    // Echo komutu için özel işlem
    if (strcmp(cmd->args[0], "echo") == 0) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Fork hatası");
            return;
        }
        
        if (pid == 0) {  // Çocuk proses
            // Çıkış yönlendirme varsa
            if (cmd->output_file != NULL) {
                int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Çıkış dosyası açılamadı");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            
            // Echo çıktısını yazdır
            for (int i = 1; i < cmd->arg_count; i++) {
                // Tırnak işaretlerini kaldır
                char *text = cmd->args[i];
                if (text[0] == '"' && text[strlen(text)-1] == '"') {
                    text[strlen(text)-1] = '\0';
                    text++;
                }
                printf("%s ", text);
            }
            printf("\n");
            exit(0);
        } else {
            if (!cmd->background) {
                waitpid(pid, NULL, 0);
            } else {
                // Arkaplan prosesi kaydet
                handle_background_process(pid, "echo");
            }
        }
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("Fork hatası");
        return;
    }
    
    if (pid == 0) {  // Çocuk proses
        // Giriş yönlendirme
        if (cmd->input_file != NULL) {
            int fd = open(cmd->input_file, O_RDONLY);
            if (fd < 0) {
                printf("%s giriş dosyası bulunamadı\n", cmd->input_file);
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        // Çıkış yönlendirme
        if (cmd->output_file != NULL) {
            int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Çıkış dosyası açılamadı");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        
        // Komutu çalıştır
        execvp(cmd->args[0], cmd->args);
        perror("Komut çalıştırma hatası");
        exit(1);
    }
    else {  // Ebeveyn proses
        if (cmd->background) {
            // Arkaplan prosesi kaydet
            char command_str[MAX_LINE] = "";
            for (int i = 0; i < cmd->arg_count; i++) {
                strcat(command_str, cmd->args[i]);
                if (i < cmd->arg_count - 1) strcat(command_str, " ");
            }
            handle_background_process(pid, command_str);
            printf("[%d] Arkaplan işlemi başlatıldı: %s\n", pid, command_str);
        }
        else {
            // Önyüz prosesi için bekle
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                printf("Komut %d çıkış koduyla sonlandı\n", WEXITSTATUS(status));
            }
        }
    }
}

void handle_background_process(pid_t pid, const char *command) {
    for (int i = 0; i < MAX_BG_PROCESSES; i++) {
        if (!bg_processes[i].active) {
            bg_processes[i].pid = pid;
            bg_processes[i].active = 1;
            strncpy(bg_processes[i].command, command, MAX_LINE - 1);
            bg_processes[i].command[MAX_LINE - 1] = '\0';
            bg_count++;
            break;
        }
    }
}

void parse_command(char *line, Command *cmd) {
    char *token;
    int i = 0;
    
    // Yapıyı sıfırla
    memset(cmd, 0, sizeof(Command));
    
    token = strtok(line, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        // Arkaplan işlemi kontrolü
        if (strcmp(token, "&") == 0) {
            cmd->background = 1;
            break;
        }
        // Giriş yönlendirme
        else if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                cmd->input_file = strdup(token);
            }
        }
        // Çıkış yönlendirme
        else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " ");
            if (token != NULL) {
                cmd->output_file = strdup(token);
            }
        }
        else {
            // Eğer tırnak işareti varsa, tüm metni al
            if (token[0] == '"') {
                char full_token[MAX_LINE] = "";
                strcat(full_token, token);
                while (token != NULL && token[strlen(token)-1] != '"') {
                    token = strtok(NULL, " ");
                    if (token != NULL) {
                        strcat(full_token, " ");
                        strcat(full_token, token);
                    }
                }
                cmd->args[i++] = strdup(full_token);
            } else {
                cmd->args[i++] = strdup(token);
            }
        }
        token = strtok(NULL, " ");
    }
    cmd->args[i] = NULL;
    cmd->arg_count = i;
}

void initialize_shell(void) {
    // Arkaplan prosesler dizisini başlat
    for (int i = 0; i < MAX_BG_PROCESSES; i++) {
        bg_processes[i].active = 0;
        bg_processes[i].command[0] = '\0';
    }
    
    // Sinyalleri ayarla
    handle_signals();
    
    // Terminal ayarlarını yap
    setvbuf(stdout, NULL, _IONBF, 0);
}

void read_command(char *line) {
    if (fgets(line, MAX_LINE, stdin) == NULL) {
        if (feof(stdin)) {
            printf("\n");
            exit(0);
        }
    }
    size_t len = strlen(line);
    if (len > 0 && line[len-1] == '\n') {
        line[len-1] = '\0';
    }
}

void cleanup_command(Command *cmd) {
    for (int i = 0; i < cmd->arg_count; i++) {
        free(cmd->args[i]);
    }
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
}

void execute_pipe_commands(char *line) {
    char *commands[MAX_ARGS];
    int cmd_count = 0;
    int pipe_count;
    int pipes[MAX_ARGS][2];
    pid_t pids[MAX_ARGS];

    // Komutları ayır ve sayısını bul
    char *token = strtok(line, "|");
    while (token != NULL && cmd_count < MAX_ARGS) {
        while (*token == ' ') token++; // Baştaki boşlukları temizle
        // Sondaki boşlukları temizle
        char *end = token + strlen(token) - 1;
        while (end > token && isspace(*end)) end--;
        *(end + 1) = 0;
        
        commands[cmd_count++] = strdup(token);
        token = strtok(NULL, "|");
    }
    pipe_count = cmd_count - 1;

    // Her komut çifti için pipe oluştur
    for (int i = 0; i < pipe_count; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("Pipe oluşturma hatası");
            return;
        }
    }

    // Her komut için process oluştur
    for (int i = 0; i < cmd_count; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("Fork hatası");
            exit(1);
        }

        if (pids[i] == 0) { // Çocuk process
            Command cmd;
            parse_command(commands[i], &cmd);

            // İlk komut değilse, önceki pipe'dan oku
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 giriş hatası");
                    exit(1);
                }
            }

            // Son komut değilse, sonraki pipe'a yaz
            if (i < pipe_count) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 çıkış hatası");
                    exit(1);
                }
            }

            // Kullanılmayan tüm pipe'ları kapat
            for (int j = 0; j < pipe_count; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // İlk komut için giriş yönlendirmesi kontrolü
            if (i == 0 && cmd.input_file != NULL) {
                int fd = open(cmd.input_file, O_RDONLY);
                if (fd < 0) {
                    printf("%s giriş dosyası bulunamadı\n", cmd.input_file);
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            // Son komut için çıkış yönlendirmesi kontrolü
            if (i == cmd_count - 1 && cmd.output_file != NULL) {
                int fd = open(cmd.output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror("Çıkış dosyası açılamadı");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            // wc komutu için özel işlem
            if (strcmp(cmd.args[0], "wc") == 0) {
                // wc komutunun parametrelerini düzelt
                if (cmd.arg_count == 3 && strcmp(cmd.args[1], "-l") == 0) {
                    cmd.args[1] = NULL;
                    cmd.arg_count = 1;
                }
            }

            // Komutu çalıştır
            execvp(cmd.args[0], cmd.args);
            perror("Komut çalıştırma hatası");
            exit(1);
        }

        free(commands[i]); // Ana process'te belleği temizle
    }

    // Ana process'te tüm pipe'ları kapat
    for (int i = 0; i < pipe_count; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Tüm çocuk process'leri bekle
    for (int i = 0; i < cmd_count; i++) {
        waitpid(pids[i], NULL, 0);
    }
}

void execute_multiple_commands(char *line) {
    char *commands[MAX_COMMANDS];
    int cmd_count = 0;
    
    // Komutları noktalı virgülle ayır
    char *token = strtok(line, ";");
    while (token != NULL && cmd_count < MAX_COMMANDS) {
        // Baştaki ve sondaki boşlukları temizle
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && isspace(*end)) end--;
        *(end + 1) = 0;
        
        if (strlen(token) > 0) {
            commands[cmd_count++] = strdup(token);
        }
        token = strtok(NULL, ";");
    }
    
    // Her komutu sırayla çalıştır
    for (int i = 0; i < cmd_count; i++) {
        // Pipe kontrolü
        if (strchr(commands[i], '|') != NULL) {
            execute_pipe_commands(commands[i]);
        } else {
            Command cmd;
            parse_command(commands[i], &cmd);
            if (cmd.arg_count > 0) {
                execute_command(&cmd);
            }
            cleanup_command(&cmd);
        }
        free(commands[i]);
    }
} 