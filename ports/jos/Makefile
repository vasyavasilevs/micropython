include ../../py/mkenv.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

PROG ?= micropython

# qstr definitions (must come before including py.mk)
QSTR_DEFS += qstrdefsport.h

# include py core make definitions
include $(TOP)/py/py.mk
include $(TOP)/extmod/extmod.mk

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

# Include JOS source tree.
INC += -I$(TOP)/../

# Debugging/Optimization
COPT ?= -Og

CFLAGS_EXTRA ?= -Wno-unused-but-set-variable

# compiler settings
CFLAGS += -fno-pic -pipe $(INC) $(CWARN) -std=gnu99 $(COPT) $(CFLAGS_EXTRA)

# Note: Symbols and debug information will still be stripped from the final binary
# unless "DEBUG=1" or "STRIP=" is passed to make, see README.md for details.
CFLAGS += -g -O1

# -fno-builtin is required to avoid refs to undefined functions in the kernel.
CFLAGS += -fno-builtin -MD -ffreestanding -fno-omit-frame-pointer -mno-red-zone
CFLAGS += -Wall -Wformat=2 -Wno-unused-function -Werror -g -gpubnames -gdwarf-4
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
# No FP registers.
CFLAGS += -mno-sse -mno-sse2 -mno-mmx
CFLAGS += -nostdlib
CFLAGS += -fno-math-errno
CFLAGS += -mcmodel=large -m64
USER_CFLAGS += -DJOS_USER

ifeq ($(MICROPY_USE_READLINE),1)
	INC += -I$(TOP)/shared/readline
	CFLAGS += -DMICROPY_USE_READLINE=1
	SHARED_SRC_C_EXTRA += readline/readline.c
endif
	ifeq ($(MICROPY_PY_TERMIOS),1)
	CFLAGS += -DMICROPY_PY_TERMIOS=1
endif
ifeq ($(MICROPY_PY_SOCKET),1)
	CFLAGS += -DMICROPY_PY_SOCKET=1
endif
ifeq ($(MICROPY_PY_THREAD),1)
	CFLAGS += -DMICROPY_PY_THREAD=1 -DMICROPY_PY_THREAD_GIL=0
endif

# source files
SRC_C += \
	alloc.c \
	fattime.c \
	gccollect.c \
	helpers.c \
	input.c \
	main.c \
	mphalport.c \
	modio.c

SRC_QSTR += shared/readline/readline.c shared/runtime/pyexec.c

OBJDIR := $(TOP)/../obj

SRC_O := $(wildcard $(OBJDIR)/lib/*.o)

SHARED_SRC_C += $(addprefix shared/,\
	runtime/gchelper_generic.c \
	timeutils/timeutils.c \
	$(SHARED_SRC_C_EXTRA) \
	)

OBJ += $(PY_O)
OBJ += $(addprefix $(BUILD)/../, $(SRC_O))
OBJ += $(addprefix $(BUILD)/, $(LIB_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SHARED_SRC_C:.c=.o))
OBJ += $(addprefix $(BUILD)/, $(SRC_C:.c=.o))

# LIBM_SRC_C += $(SRC_LIB_LIBM_DBL_C)
# ifeq ($(SUPPORTS_HARDWARE_FP_DOUBLE),1)
#     LIBM_SRC_C += $(SRC_LIB_LIBM_DBL_SQRT_HW_C)
# else
#     LIBM_SRC_C += $(SRC_LIB_LIBM_DBL_SQRT_SW_C)
# endif
# OBJ += $(addprefix $(BUILD)/, $(LIBM_SRC_C:.c=.o))

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SHARED_SRC_C)

ifneq ($(FROZEN_MANIFEST),)
# force 16 bits to work on both 32 and 64 bit archs
CFLAGS += -DMPZ_DIG_SIZE=16
endif

LIBS =

LDFLAGS += $(LDFLAGS_MOD) $(LDFLAGS_ARCH) $(LDFLAGS_EXTRA)

# Linker flags for JOS programs
USER_LD_SCRIPT = $(TOP)/../user/user.ld
LDFLAGS += -Wl,-T $(USER_LD_SCRIPT)

LDFLAGS += -Wl,-m elf_x86_64
LDFLAGS += -Wl,-z max-page-size=0x1000
LDFLAGS += -Wl,-z noexecstack
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--warn-common
LDFLAGS += -Wl,-nostdlib
LDFLAGS += -nostdlib
LDFLAGS += -Wl,-Map=$@.map,--cref
LDFLAGS += -Wl,-no-pie

OBJDIR_LIB_PATH := $(OBJDIR)/lib
LDFLAGS += -Wl,$(OBJDIR_LIB_PATH)/libjos.a

LIBGCC_FILE_NAME = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
LDFLAGS += -Wl,$(LIBGCC_FILE_NAME)

# ARCH := $(shell uname -m)
# ARCH_LINUX_GNU_PATH := /usr/lib/$(ARCH)-linux-gnu
# LDFLAGS += -Wl,-L$(ARCH_LINUX_GNU_PATH)
LDFLAGS += -Wl,-static
CFLAGS += -static
# LDFLAGS += -Wl,-l:libm.a
# CFLAGS += -D_DL_X86_CPU_FEATURES__WORKAROUND
# LDFLAGS += -Wl,-lm
# LDFLAGS += -Wl,$(ARCH_LINUX_GNU_PATH)/libm.a

ifeq ($(MICROPY_FORCE_32BIT),1)
RUN_TESTS_MPY_CROSS_FLAGS = --mpy-cross-flags='-march=x86'
endif

include $(TOP)/py/mkrules.mk
