.SUFFIXES: .c .o

COLDFIRE_BASE_DIR = e:/apps/lang/coldfire
#COLDFIRE_BASE_DIR = c:/coldfire
COLDFIRE_EXE_DIR = $(COLDFIRE_BASE_DIR)/gcc-m68k/bin

CC = $(COLDFIRE_EXE_DIR)/m68k-coff-gcc.exe
OBJCOPY = $(COLDFIRE_EXE_DIR)/m68k-coff-objcopy.exe

SRC_DIR = .
OBJ_DIR = $(SRC_DIR)
OUT_DIR = $(SRC_DIR)

CC_FLAGS = \
  -c \
  -m68332 \
  -I$(SRC_DIR) \
  -I$(COLDFIRE_BASE_DIR)/gcc-m68k/m68k-coff/include \
  -I$(COLDFIRE_BASE_DIR)/gcc-m68k/lib/gcc-lib/m68k-coff/egcs-2.91.10/include \
  -gcoff \
  -gcoff3 \
  -O1 \
  -funsigned-char \
  -mno-align-int \
  -Wcast-align \
  -Wimplicit \
  -Wmissing-declarations \
  -Wmissing-prototypes \
  -Wnested-externs \
  -Wold-style-cast \
  -Wparentheses \
  -Wpointer-arith \
  -Wreturn-type \
  -Wsign-compare \
  -Wstrict-prototypes \
  -Wswitch \
  -Wundef \
  -Wunused \
  -Wwrite-strings \
  -W \

LINK_FLAGS = \
  -m68332 \
  -nostartfiles \
  --warn-common \
  -Wl,-n \
  -Wl,-M \
  -Wl,--cref \


BOOTSTUB_OBJECTS = \
  $(OBJ_DIR)/mainboot.o \
  $(OBJ_DIR)/gdb.o \
  $(OBJ_DIR)/gdbcpu32.o \

TESTAPP_OBJECTS = \
  $(OBJ_DIR)/mainapp.o \
  $(OBJ_DIR)/testapp.o \
  $(OBJ_DIR)/gdb.o \
  $(OBJ_DIR)/gdbcpu32.o \


all : $(OUT_DIR)/bootstub.cof $(OUT_DIR)/testapp.cof $(OUT_DIR)/bootstub.s19


$(OUT_DIR)/bootstub.cof : $(BOOTSTUB_OBJECTS) $(SRC_DIR)/bootstub.ld
	$(CC) $(LINK_FLAGS) -T$(SRC_DIR)/bootstub.ld -Wl,-Map=$(OUT_DIR)/bootstub.map $(BOOTSTUB_OBJECTS) -o $@

$(OUT_DIR)/testapp.cof : $(TESTAPP_OBJECTS) $(SRC_DIR)/testapp.ld
	$(CC) $(LINK_FLAGS) -T$(SRC_DIR)/testapp.ld -Wl,-Map=$(OUT_DIR)/testapp.map $(TESTAPP_OBJECTS) -o $@


$(OUT_DIR)/bootstub.s19 : $(BOOTSTUB_OBJECTS) $(SRC_DIR)/bootstub.ld
	$(OBJCOPY) -O srec $(OUT_DIR)/$(notdir $(basename $@)).cof $@


$(OBJ_DIR)/mainboot.o : main.c
	$(CC) -c $< $(CC_FLAGS) -o $@

$(OBJ_DIR)/mainapp.o : main.c
	$(CC) -c -DTESTAPP $< $(CC_FLAGS) -o $@


$(OBJ_DIR)/testapp.o : testapp.c
	$(CC) -c $< $(CC_FLAGS) -o $@

$(OBJ_DIR)/gdb.o : gdb.c
	$(CC) -c $< $(CC_FLAGS) -o $@

$(OBJ_DIR)/gdbcpu32.o : gdbcpu32.c
	$(CC) -c $< $(CC_FLAGS) -o $@

