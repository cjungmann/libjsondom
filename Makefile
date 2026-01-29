TARGET_ROOT = jsondom
TARGET_SHARED = lib${TARGET_ROOT}.so
TARGET_STATIC = lib${TARGET_ROOT}.a
TARGET_TEST = test

PREFIX ?= /usr/local
MAN_SECTION = 3
MAN_PATH = $(PREFIX)/share/man/man$(MAN_SECTION)

MAN_PAGE = $(TARGET_ROOT).$(MAN_SECTION)

# Change if source files not in base directory:
SRC = .

CFLAGS = -Wall -Werror -std=c99 -pedantic -ggdb -fvisibility=hidden
LFLAGS =
LDFLAGS =

# CFLAGS += -fsanitize=address
# LDFLAGS += -fsanitize=address

# Uncomment the following if target is a Shared library
CFLAGS += -fPIC
LFLAGS += --shared

TEST_LIBS =  -lcontools -ltinfo

# Build module list (info make -> "Functions" -> "File Name Functions")
MODULES = $(addsuffix .o,$(filter-out ./test_%,$(basename $(wildcard $(SRC)/*.c))))
TEST_TARGETS = $(subst test_,,$(filter ./test_%,$(basename $(wildcard $(SRC)/*.c))))
TEST_SOURCES = $(addsuffix .c,$(filter ./test_%,$(basename $(wildcard $(SRC)/*.c))))
TEST_MODULES = $(addsuffix .o,$(filter ./test_%,$(basename $(wildcard $(SRC)/*.c))))

# Libraries need header files.  Set the following accordingly:
HEADERS = $(TARGET_ROOT).h

define prereq_check_setup =
	@echo "Pre-compile dependencies test"
	@rm -f PREREQ_CHECK.REPORT
endef

define prereq_check_library =
	@echo "Checking for $(1) library"
	@/sbin/ldconfig -p | grep -o ^[[:space:]]*lib$(1).so >/dev/null; \
		echo "$$?" > PREREQ_CHECK_LIBRARY.RESULT
	@if grep -v 0 PREREQ_CHECK_LIBRARY.RESULT > /dev/null; then              \
		printf '- Missing library \e[32;1m%s\e[m (\e[32;1m%s\e[m)\n' \
			$(1) $(2) >> PREREQ_CHECK.REPORT; \
	fi
	@rm -f PREREQ_CHECK_LIBRARY.RESULT
endef

define prereq_check_header =
	@echo "Checking include path for header $(1)"
	@echo "#include <$(1)>" > PREREQ_CHECK_DUMMY.h
	@-$(CC) -fsyntax-only PREREQ_CHECK_DUMMY.h 1>&2 2>/dev/null; \
		echo "$$?" > PREREQ_CHECK_HEADER.RESULT
	@if grep -v 0 PREREQ_CHECK_HEADER.RESULT > /dev/null; then          \
		printf ' - Missing header \e[32;1m%s\e[m; install \e[32;1m%s\e[m.\n' \
			$(1) $(2) >> PREREQ_CHECK.REPORT; \
	fi
	@rm -f PREREQ_CHECK_DUMMY.h PREREQ_CHECK_HEADER.RESULT
endef

define prereq_check_report =
	@if [ -f PREREQ_CHECK.REPORT ] && \
			[ $$( stat -c%s PREREQ_CHECK.REPORT ) -ne 0 ]; then \
		echo ; \
		echo "Predicting compile failure due to missing installations:" ; \
		cat PREREQ_CHECK.REPORT; \
		rm -f PREREQ_CHECK.REPORT;  \
		exit 1;                  \
	fi
	@rm -f PREREQ_CHECK.REPORT
endef

# Declare non-filename targets
.PHONY: all preview install uninstall clean help depends

all: depends ${TARGET_SHARED} ${TARGET_STATIC}

depends:
	$(call prereq_check_setup)
	$(call prereq_check_library,ncurses,na)
	$(call prereq_check_header,ncurses.h,libncurses5-dev)
	$(call prereq_check_report)

preview:
	@echo Beginning build of ${TARGET_ROOT} libraries.
	@echo "shared library:" ${TARGET_SHARED}
	@echo "static library:" ${TARGET_STATIC}
	@echo "modules:       " ${MODULES}
	@echo "test sources:  " $(TEST_SOURCES)
	@echo "test targets:  " $(TEST_TARGETS)

${TARGET_SHARED}: ${MODULES} ${HEADERS}
	${CC} ${CFLAGS} --shared -o $@ ${MODULES}

${TARGET_STATIC}: ${MODULES} ${HEADERS}
	ar rcs $@ ${MODULES}

%o : %c ${HEADERS}
	$(CC) $(CFLAGS) -c -o $@ $<

test:
	rm -f $(TEST_TARGETS)
	$(MAKE) $(TEST_TARGETS)

$(TEST_TARGETS) : $(TEST_SOURCES)
	$(CC) $(CFLAGS) -o $@ test_$@.c $(TARGET_STATIC) $(TEST_LIBS)

For shared library targets:
install:
	mkdir --mode=775 -p $(MAN_PATH)
	install -D --mode=644 $(HEADERS) $(PREFIX)/include
	install -D --mode=775 $(TARGET_STATIC) $(PREFIX)/lib
	install -D --mode=775 $(TARGET_SHARED) $(PREFIX)/lib
	soelim $(TARGET_ROOT).$(MAN_SECTION) | gzip -c - > $(MAN_PATH)/$(TARGET_ROOT).$(MAN_SECTION).gz
	soelim $(TARGET_ROOT)_screen.$(MAN_SECTION) | gzip -c - > $(MAN_PATH)/$(TARGET_ROOT)_screen.$(MAN_SECTION).gz
	ldconfig $(PREFIX)/lib

# Remove the ones you don't need:
uninstall:
	rm -f $(PREFIX)/lib/$(TARGET)
	rm -f $(PREFIX)/include/$(HEADERS)
	rm -f $(MAN_PATH)/$(TARGET_ROOT).$(MAN_SECTION).gz
	rm -f $(MAN_PATH)/$(TARGET_ROOT)_screen.$(MAN_SECTION).gz
	ldconfig $(PREFIX)/lib

clean:
	rm -f $(TARGET_SHARED)
	rm -f $(TARGET_STATIC)
	rm -f $(TARGET_TEST)
	rm -f $(MODULES)
	rm -f $(TEST_TARGETS)

help:
	@echo "Makefile options:"
	@echo
	@echo "  test       to build test program using library"
	@echo "  preview    to see relevent files"
	@echo "  install    to install project"
	@echo "  uninstall  to uninstall project"
	@echo "  clean      to remove generated files"
	@echo "  help       this display"
