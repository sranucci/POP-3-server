STANDARD= -std=c11
DISABLE_STYLE = -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-fallthrough -Wno-newline-eof  # disable style warnings like unused functions
DEBUG = -g # enable debugging symbols
STRICT = -Werror # treat warnings as errors
SANITIZERS = -fsanitize=address -fsanitize=undefined -fsanitize=leak
CFLAGS = $(STANDARD) $(DISABLE_STYLE) $(DEBUG) $(STRICT) $(SANITIZERS) -pedantic -pedantic-errors -Wall -Wextra -D_POSIX_C_SOURCE=200112L -Wuninitialized -Wunused-variable 
SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES = $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

TARGET = $(BINDIR)/project

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJECTS): | $(OBJDIR)
$(TARGET): | $(BINDIR)

$(OBJDIR) $(BINDIR):
	@mkdir -p $@


.PHONY: clean

clean:
	rm -rf $(OBJDIR) $(BINDIR) 
	$(MAKE) -C Client-interface clean

all: $(TARGET) 
	$(MAKE) -C Client-interface all

.DEFAULT_GOAL := all


# -g                    debug information
# -Wall                    enables a set of commonly used warnings. It includes warnings about common programming issues and potential bugs.
# -Wextra                enables additional warnings beyond those enabled by -Wall. It covers a wider range of potential issues and encourages stricter coding practices.
# -pedantic                enables strict ISO C and ISO C++ conformance. It provides warnings about language features that are non-standard or prone to portability issues.
# -Werror                treats warnings as errors. It forces the compilation to fail if any warnings are encountered, making it useful for enforcing clean code.
# -Wuninitialized         warns about the use of uninitialized variables
# -Wunused-variable        warns about unused variables
# -fsanitize=address    Enables the AddressSanitizer, which helps detect memory errors such as buffer overflows, use-after-free, and memory leaks.
# -fsanitize=undefined    Enables the UndefinedBehaviorSanitizer, which helps detect undefined behavior in your code.
# -fsanitize=leak        Enables the LeakSanitizer, which helps detect memory leaks.