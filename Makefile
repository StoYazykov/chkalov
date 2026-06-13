CXX = g++
CC = gcc

CXXFLAGS = -std=c++17 -w -O2
CFLAGS = -w -O2 -fPIC
LDFLAGS = -ldl

CHKALOV_DIR = /root/chk
STDCON_DIR = $(CHKALOV_DIR)/stdcon
CHKALOV_SRC = $(CHKALOV_DIR)/chkalov
VM_DIR = $(CHKALOV_DIR)/chkalovm

PARSER_SRC = $(CHKALOV_SRC)/parser.cpp $(CHKALOV_SRC)/scope.cpp $(CHKALOV_SRC)/token.cpp
MAIN_SRC = $(CHKALOV_SRC)/main.cpp
STDCON_SRC = $(STDCON_DIR)/main.c
VM_SRC = $(VM_DIR)/main.cpp

CHKC = $(CHKALOV_DIR)/chkc
CHKVM = $(CHKALOV_DIR)/chkvm
LIBSTDCON = $(STDCON_DIR)/libstdcon.so

PARSER_OBJ = $(PARSER_SRC:.cpp=.o)
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
VM_OBJ = $(VM_SRC:.cpp=.o)

DEMO_SRC = $(CHKALOV_DIR)/demo.txt
DEMO_CVM = $(CHKALOV_DIR)/demo.cvm

all: $(CHKC) $(CHKVM) $(LIBSTDCON)
	@echo "=============================="
	@echo "         Компиляция...        "
	@echo "=============================="
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) && $(CHKVM) $(DEMO_CVM)

$(CHKC): $(MAIN_OBJ) $(PARSER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	

$(CHKVM): $(VM_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	
$(LIBSTDCON): $(STDCON_SRC)
	$(CC) $(CFLAGS) -shared -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(CHKC) $(CHKVM) $(LIBSTDCON)
	rm -f $(PARSER_OBJ) $(MAIN_OBJ) $(VM_OBJ)
	rm -f $(DEMO_CVM)

build: $(CHKC) $(CHKVM) $(LIBSTDCON)

run: build
	$(CHKC) $(DEMO_SRC) $(DEMO_CVM) && $(CHKVM) $(DEMO_CVM)

rebuild: clean all

info:
	@echo "Чкалов — быстрейший язык программирования!"
	@echo "Компилятор: $(CHKC)"
	@echo "VM:         $(CHKVM)"
	@echo "Библиотека: $(LIBSTDCON)"

.PHONY: all clean build run rebuild info
