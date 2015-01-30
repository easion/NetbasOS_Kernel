ifeq ($(MACHINE),athlon-xp)
	CFLAGS += -march=athlon-xp -mmmx -msse -m3dnow
	DEFS += -DCONFIG_FENCES_P3
	CONFIG_SMP = n
	CONFIG_HT = n
endif

ifeq ($(MACHINE),athlon-mp)
	CFLAGS += -march=athlon-mp -mmmx -msse -m3dnow
	DEFS += -DCONFIG_FENCES_P3
	CONFIG_HT = n
endif

ifeq ($(MACHINE),pentium3)
	CFLAGS += -march=pentium3 -mmmx -msse
	DEFS += -DCONFIG_FENCES_P3
	CONFIG_HT = n
endif

ifeq ($(MACHINE),prescott)
	CFLAGS += -march=pentium4 -mfpmath=sse -mmmx -msse -msse2 -msse3
	DEFS += -DCONFIG_FENCES_P4
endif

ifeq ($(MACHINE),pentium4)
	CFLAGS += -march=pentium4 -mfpmath=sse -mmmx -msse -msse2
	DEFS += -DCONFIG_FENCES_P4
endif

