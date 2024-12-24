# Talha Kenan Yaylacık B211210099 1C
# Beyzanur Karaçam G211210054 2C

# Derleyici ve bayraklar
CC = gcc
CFLAGS = -Wall -Wextra

# Hedef dosya
TARGET = myshell

# Kaynak dosyalar
SOURCES = main.c shell.c
HEADERS = shell.h

# Nesne dosyaları
OBJECTS = $(SOURCES:.c=.o)

# Varsayılan hedef
all: $(TARGET)

# Programı derle
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Nesne dosyalarını oluştur
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Temizlik
clean:
	rm -f $(OBJECTS) $(TARGET)

# Yeniden derle
rebuild: clean all

# Çalıştır
run: $(TARGET)
	./$(TARGET)

# Hedefleri .PHONY olarak işaretle
.PHONY: all clean rebuild run 