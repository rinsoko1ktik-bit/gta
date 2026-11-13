# Makefile - يبني zokii_mod_menu.prx بـ OpenOrbis + GoldHEN Plugins SDK
# المتغيرات تجي من الـ workflow: OO_PS4_TOOLCHAIN, GOLDHEN_SDK, LUA_DIR
PLUGIN := zokii_mod_menu
CC := clang
CXX := clang++
INC := -I. -I$(LUA_DIR) -I$(GOLDHEN_SDK)/include -I$(OO_PS4_TOOLCHAIN)/include -I$(OO_PS4_TOOLCHAIN)/include/c++/v1
CFLAGS := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -O2 $(INC) -DLUA_USE_POSIX
CXXFLAGS := $(CFLAGS) -std=c++17 -fno-rtti -fno-exceptions
LDFLAGS := -m elf_x86_64 -pie --script $(OO_PS4_TOOLCHAIN)/link.x --eh-frame-hdr -L$(OO_PS4_TOOLCHAIN)/lib -L$(GOLDHEN_SDK) -lc -lc++ -lkernel -lScePad -lGoldHEN_Hook

LUA_SRC := $(filter-out $(LUA_DIR)/lua.c $(LUA_DIR)/luac.c, $(wildcard $(LUA_DIR)/*.c))
OBJS := main.o $(LUA_SRC:.c=.o)

all: $(PLUGIN).prx

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PLUGIN).elf: $(OBJS)
	ld.lld $(OBJS) $(LDFLAGS) -o $@

$(PLUGIN).prx: $(PLUGIN).elf
	$(OO_PS4_TOOLCHAIN)/bin/linux/create-fself -in=$< -out=$(PLUGIN).oelf --lib=$@ --paid 0x3800000000000011

clean:
	rm -f *.o $(LUA_DIR)/*.o *.elf *.oelf *.prx
