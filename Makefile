CXX = g++
CC = gcc

CXXFLAGS = -std=c++17 -w -O2
CFLAGS = -w -O2 -fPIC
LDFLAGS = -ldl

CHKALOV_DIR = /root/chk
STDCON_DIR = $(CHKALOV_DIR)/stdcon
CHKALOV_SRC = $(CHKALOV_DIR)/chkalov
VM_DIR = $(CHKALOV_DIR)/chkalovm
ITT_DIR = $(CHKALOV_DIR)/chkalovitt

PARSER_SRC = $(CHKALOV_SRC)/parser.cpp $(CHKALOV_SRC)/scope.cpp $(CHKALOV_SRC)/token.cpp
MAIN_SRC = $(CHKALOV_SRC)/main.cpp
STDCON_SRC = $(STDCON_DIR)/main.c
VM_SRC = $(VM_DIR)/main.cpp
ITT_SRC = $(ITT_DIR)/main.cpp

CHKC = $(CHKALOV_DIR)/chkc
CHKVM = $(CHKALOV_DIR)/chkvm
LIBSTDCON = $(STDCON_DIR)/libstdcon.so
ITT = $(CHKALOV_DIR)/chkalovitt

PARSER_OBJ = $(PARSER_SRC:.cpp=.o)
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
VM_OBJ = $(VM_SRC:.cpp=.o)
ITT_OBJ = $(ITT_SRC:.cpp=.o)

DEMO_SRC = $(CHKALOV_DIR)/demo.txt
DEMO_CVM = $(CHKALOV_DIR)/demo.cvm

# ============ ОБЫЧНАЯ СБОРКА (релиз) ============
all: $(CHKC) $(CHKVM) $(LIBSTDCON) $(ITT)
	@echo "=============================="
	@echo "         Компиляция...        "
	@echo "=============================="
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) && $(CHKVM) $(DEMO_CVM)

# ============ ОТЛАДОЧНЫЙ ЗАПУСК ============
debug: $(CHKC) $(CHKVM) $(LIBSTDCON) $(ITT)
	@echo "=============================="
	@echo "      Debug-компиляция...     "
	@echo "=============================="
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) -d && $(CHKVM) $(DEMO_CVM) -d

# ============ СБОРКА КОМПОНЕНТОВ ============
$(CHKC): $(MAIN_OBJ) $(PARSER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(CHKVM): $(VM_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(LIBSTDCON): $(STDCON_SRC)
	$(CC) $(CFLAGS) -shared -o $@ $^

$(ITT): $(ITT_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# ============ ОЧИСТКА ============
clean:
	rm -f $(CHKC) $(CHKVM) $(LIBSTDCON) $(ITT)
	rm -f $(PARSER_OBJ) $(MAIN_OBJ) $(VM_OBJ) $(ITT_OBJ)
	rm -f $(DEMO_CVM)

# ============ ПРОЧЕЕ ============
build: $(CHKC) $(CHKVM) $(LIBSTDCON) $(ITT)

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
