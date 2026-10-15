CC = gcc
CFLAGS = -std=c99 -w -O2
LDFLAGS = -ldl

ifeq ($(TARGET),win)
EXT = .exe
LIBSTDCON = $(STDCON_DIR)/stdcon.dll
CFLAGS=$(CFLAGS) -fno-use-linker-plugin
else
EXT =
LIBSTDCON = $(STDCON_DIR)/libstdcon.so
endif

CHKALOV_DIR = /root/c
STDCON_DIR = $(CHKALOV_DIR)/stdcon
CHKALOV_SRC = $(CHKALOV_DIR)/chkalov
VM_DIR = $(CHKALOV_DIR)/chkalovm
DIS_DIR = $(CHKALOV_DIR)/chkalovdis
#ITT_DIR = $(CHKALOV_DIR)/chkalovitt

COMPILER_SRC = $(CHKALOV_SRC)/main.c \
$(CHKALOV_SRC)/parser/parser.c \
$(CHKALOV_SRC)/scope/scope.c \
$(CHKALOV_SRC)/token/token.c \
$(CHKALOV_DIR)/dynamic-strings/ds.c \
$(CHKALOV_DIR)/c-vectors/cv.c \
$(CHKALOV_DIR)/chkalov.c \
$(CHKALOV_SRC)/ast/ast.c \
$(CHKALOV_SRC)/codegen/codegen.c \
$(CHKALOV_SRC)/lexer/lexer.c \
$(CHKALOV_SRC)/native/native.c \
$(CHKALOV_SRC)/function/function.c \
$(CHKALOV_SRC)/sema/sema_check.c

STDCON_SRC = $(STDCON_DIR)/main.c
VM_SRC = $(VM_DIR)/main.c \
$(CHKALOV_DIR)/dynamic-strings/ds.c \
$(CHKALOV_DIR)/c-vectors/cv.c \
$(CHKALOV_DIR)/chkalov.c
DIS_SRC = $(DIS_DIR)/main.c \
$(CHKALOV_DIR)/chkalov.c \
$(CHKALOV_DIR)/c-vectors/cv.c

CHKC = $(CHKALOV_DIR)/chkc$(EXT)
CHKVM = $(CHKALOV_DIR)/chkvm$(EXT)
LIBSTDCON = $(STDCON_DIR)/libstdcon.so
DIS = $(CHKALOV_DIR)/chkdis$(EXT)

COMPILER_OBJ = $(COMPILER_SRC:.c=.o)
VM_OBJ = $(VM_SRC:.c=.o)

DEMO_SRC = $(CHKALOV_DIR)/demo.chk
DEMO_CVM = $(CHKALOV_DIR)/demo.cvm

all: $(CHKC) $(CHKVM) $(LIBSTDCON) $(DIS)
	@echo "=============================="
	@echo "         Компиляция...        "
	@echo "=============================="
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) && echo "" && echo "-- VM --" && echo "" && $(CHKVM) $(DEMO_CVM)

debug: CFLAGS = -std=c99 -w -g -O0
debug: clean all
	@echo "=============================="
	@echo "      Debug-компиляция...     "
	@echo "=============================="
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) -d && echo "" && echo "-- VM -- " && echo "" && $(CHKVM) $(DEMO_CVM) -d

win:
	make TARGET=win rebuild


$(CHKC): $(COMPILER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CHKVM): $(VM_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(LIBSTDCON): $(STDCON_SRC)
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^
$(DIS): $(DIS_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

#$(ITT): $(ITT_SRC)
#	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(CHKC) $(CHKVM) $(LIBSTDCON)
	rm -f $(COMPILER_OBJ) $(VM_OBJ)
	rm -f $(DEMO_CVM)

build: $(CHKC) $(CHKVM) $(LIBSTDCON) $(DIS) #$(ITT)

run: build
	@export CHKALOV=$(CHKALOV_DIR) && $(CHKC) $(DEMO_SRC) $(DEMO_CVM) && $(CHKVM) $(DEMO_CVM)

rebuild: clean all

.PHONY: all clean build run rebuild info debug
