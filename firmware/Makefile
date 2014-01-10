### Project name (also used for output file name)
PROJECT	= winter_project

### Source files in ARM or Thumb(-2)
CSRC	= $(shell find -name "*.c")
ASRC	= nvol_section.S

### Source files in ARM-only
CSRCARM	=
ASRCARM	=

### Optimization level (0, 1, 2, 3, 4 or s)
OPTIMIZE = s

### C Standard level (c89, gnu89, c99 or gnu99)
CSTD = gnu99

### Output file format (ihex, bin or both) and debugger type
OUTPUT	= ihex
DEBUG	= dwarf-2

### Processor type and Thumb(-2) mode for CSRC/ASRC files (YES or NO)
CPU   = cortex-m0
THUMB = YES

### Linker script for the target MCU
LINKSCRIPT = lpc11c24.ld

### Include dirs, library dirs and definitions
MATHLIB	= -lm
LIBS	=
LIBDIRS	=
INCDIRS	=
DEFS	=
ADEFS	=

### Warning contorls
WARNING	= all extra


### Programs to build porject
TOOLSPREFIX =  /usr/local/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi-
CC      = $(TOOLSPREFIX)gcc
OBJCOPY = $(TOOLSPREFIX)objcopy
OBJDUMP = $(TOOLSPREFIX)objdump
SIZE    = $(TOOLSPREFIX)size
NM      = $(TOOLSPREFIX)nm




### Compiler flags

ifeq ($(THUMB),YES)
THUMBFLAG = -mthumb
THUMBIW = -mthumb-interwork
else
THUMBFLAG =
THUMBIW =
endif

# Flags for C files
CFLAGS += -std=$(CSTD)
CFLAGS += -g$(DEBUG)
CFLAGS += -O$(OPTIMIZE)
CFLAGS += $(patsubst %,-W%,$(WARNING))
CFLAGS += $(patsubst %,-I%,$(INCDIRS))
CFLAGS += $(DEFS)
CFLAGS += -Icmsis/inc
CFLAGS += -DAHB_CLOCK=12000000
# used in flash_nvol.c
CFLAGS += -DCPU_CLK=12000
#CFLAGS += -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(*F).d


# Assembler flags
ASFLAGS += $(ADEFS) -Wa,-g$(DEBUG)


# Linker flags
LDFLAGS += -nostartfiles -Wl,-Map=$(PROJECT).map,--cref,--gc-sections
LDFLAGS += -lc -lgcc
LDFLAGS += $(patsubst %,-L%,$(LIBDIRS)) $(patsubst %,-l%,$(LIBS))
LDFLAGS += $(MATHLIB)
LDFLAGS += -T$(LINKSCRIPT)


# Define all object files
COBJ      = $(CSRC:.c=.o)
AOBJ      = $(ASRC:.S=.o)
COBJARM   = $(CSRCARM:.c=.o)
AOBJARM   = $(ASRCARM:.S=.o)

# Define all listing files
LST = $(PROJECT).lst $(ASRC:.S=.lst) $(ASRCARM:.S=.lst) $(CSRC:.c=.lst) $(CSRCARM:.c=.lst)


# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS  = -mcpu=$(CPU) $(THUMBIW) -I. $(CFLAGS)
ALL_ASFLAGS = -mcpu=$(CPU) $(THUMBIW) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: version build size

ifeq ($(OUTPUT),ihex)
build: elf hex lst sym
hex: $(PROJECT).hex
else
ifeq ($(OUTPUT),binary)
build: elf bin lst sym
bin: $(PROJECT).bin
else
ifeq ($(OUTPUT),both)
build: elf hex bin lst sym
hex: $(PROJECT).hex
bin: $(PROJECT).bin
else
$(error "Invalid format: $(OUTPUT)")
endif
endif
endif

elf: $(PROJECT).elf
lst: $(PROJECT).lst
sym: $(PROJECT).sym


# Display compiler version information.
version :
	@$(CC) --version

# Create final output file (.hex or .bin) from ELF output file.
%.hex: %.elf
	@echo
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	@echo
	$(OBJCOPY) -O binary $< $@

# Create extended listing file from ELF output file.
%.lst: %.elf
	@echo
	$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	$(NM) -n $< > $@

# Display size of file.
size:
	@echo
	$(SIZE) -A $(PROJECT).elf


# Link: create ELF output file from object files.
.SECONDARY : $(PROJECT).elf
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ)
%.elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ)
	@echo
	@echo Linking...
	$(CC) $(THUMBFLAG) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM or Thumb(-2)
$(COBJ) : %.o : %.c
	@echo
	@echo $< :
	$(CC) -c $(THUMBFLAG) $(ALL_CFLAGS) $< -o $@

# Compile: create object files from C source files. ARM-only
$(COBJARM) : %.o : %.c
	@echo
	@echo $< :
	$(CC) -c $(ALL_CFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM or Thumb(-2)
$(AOBJ) : %.o : %.S
	@echo
	@echo $< :
	$(CC) -c $(THUMBFLAG) $(ALL_ASFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : %.o : %.S
	@echo
	@echo $< :
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
clean:
	@echo
	rm -f $(PROJECT).hex $(PROJECT).bin $(PROJECT).elf $(PROJECT).map $(PROJECT).sym
	rm -f $(COBJ) $(AOBJ) $(COBJARM) $(AOBJARM)
	rm -f $(LST)
	rm -rf libcan
#	rm -f $(CSRC:.c=.s) $(CSRCARM:.c=.s)
	rm -f -r .dep | exit 0

.PNOHY:write
write: $(PROJECT).hex
# 	/usr/local/lpc21isp/lpc21isp -control $(PROJECT).hex /dev/ttyUSB0 115200 12000000
	lpc21isp -control $(PROJECT).hex /dev/ttyUSB0 115200 12000000

# Include the dependency files.
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*.d)

