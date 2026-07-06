CC = gcc
CFLAGS = -std=c11 -w -O2
LDFLAGS = -ldl

CHKALOV_DIR = /root/c
STDCON_DIR = $(CHKALOV_DIR)/stdcon
CHKALOV_SRC = $(CHKALOV_DIR)/chkalov
VM_DIR = $(CHKALOV_DIR)/chkalovm
#ITT_DIR = $(CHKALOV_DIR)/chkalovitt

# Исходники компилятора
COMPILER_SRC = $(CHKALOV_SRC)/main.c \
               $(CHKALOV_SRC)/parser.c \
               $(CHKALOV_SRC)/scope.c \
               $(CHKALOV_SRC)/token.c \
               $(CHKALOV_DIR)/ds.c \
               $(CHKALOV_DIR)/cv.c \
	       $(CHKALOV_DIR)/chkalov.c

STDCON_SRC = $(STDCON_DIR)/main.c
VM_SRC = $(VM_DIR)/main.c \
	 $(CHKALOV_DIR)/ds.c \
	 $(CHKALOV_DIR)/cv.c \
	 $(CHKALOV_DIR)/chkalov.c
#ITT_SRC = $(ITT_DIR)/main.c

# Выходные файлы
CHKC = $(CHKALOV_DIR)/chkc
CHKVM = $(CHKALOV_DIR)/chkvm
LIBSTDCON = $(STDCON_DIR)/libstdcon.so
#ITT = $(CHKALOV_DIR)/chkalovitt

# Объектные файлы
COMPILER_OBJ = $(COMPILER_SRC:.c=.o)
VM_OBJ = $(VM_SRC:.c=.o)
#ITT_OBJ = $(ITT_SRC:.c=.o)

# Демо-файлы
DEMO_SRC = $(CHKALOV_DIR)/demo.txt
DEMO_CVM = $(CHKALOV_DIR)/demo.cvm

# ============ ОБЫЧНАЯ СБОРКА ============
all: $(CHKC) $(CHKVM) $(LIBSTDCON) $(ITT)
	@echo "=============================="
	@echo "         Компиляция...        "
	@echo "=============================="
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) && $(CHKVM) $(DEMO_CVM)

# ============ ОТЛАДОЧНЫЙ ЗАПУСК ============
debug: CFLAGS = -std=c11 -w -g -O0
debug: clean all
	@echo "=============================="
	@echo "      Debug-компиляция...     "
	@echo "=============================="
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) -d && $(CHKVM) $(DEMO_CVM) -d

# ============ СБОРКА КОМПОНЕНТОВ ============
$(CHKC): $(COMPILER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CHKVM): $(VM_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(LIBSTDCON): $(STDCON_SRC)
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^

#$(ITT): $(ITT_SRC)
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ============ ОЧИСТКА ============
clean:
	rm -f $(CHKC) $(CHKVM) $(LIBSTDCON)
	rm -f $(COMPILER_OBJ) $(VM_OBJ)
	rm -f $(DEMO_CVM)

# ============ ПРОЧЕЕ ============
build: $(CHKC) $(CHKVM) $(LIBSTDCON) #$(ITT)

run: build
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) && $(CHKVM) $(DEMO_CVM)

rebuild: clean all

info:
	@echo "Чкалов — быстрейший язык программирования!"
	@echo "Компилятор: $(CHKC)"
	@echo "VM:         $(CHKVM)"
	@echo "Библиотека: $(LIBSTDCON)"
	@echo "Инструменты: $(ITT)"

.PHONY: all clean build run rebuild info debug
