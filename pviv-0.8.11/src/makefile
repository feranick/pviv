# use "gcc" to compile source files.
CC = g++
# the linker is also "gcc". It might be something else with other compilers.
LD = g++
# Default Compiler flags: dynamic build
#CFLAGS = -Wall -O2 -fomit-frame-pointer -pipe -lgrace_np
#LDFLAGS = -Wall -s -O2
#---------------------------------
# Default Compiler flags: static build
CFLAGS = -Wall -O3 -fomit-frame-pointer -pipe -static -lgrace_np
LDFLAGS = -Wall -s -O3 -static
#---------------------------------
#CLFAGS for Intel Atom
#CHOST=i686-pc-linux-gnu
#CFLAGS=-O2 -march=core2 -mtune=native -fomit-frame-pointer -pipe
#CXXFLAGS=${CFLAGS}
#---------------------------------
#CFLAGS = -Wall -O2 -march=native
#CFLAGS = -O2 -march=native -pipe -fomit-frame-pointer -mfpmath=sse -msse -msse2 -msse3 -mmmx -m3dnow
# Linker flags go here. Currently there aren't any, but if we'll switch to
# code optimization, we might add "-s" here to strip debug info and symbols.
#LDFLAGS = -Wall -s -O2 -march=core2
#LDFLAGS = -Wall -s -O2 -march=native
#LDFLAGS = -O2 -march=native -pipe -fomit-frame-pointer -mfpmath=sse -msse -msse2 -msse3 -mmmx -m3dnow
# use this command to erase files.
RM = rm -f 
# list of souces
SOURCE = pviv.cpp
# list of generated object files.
OBJS = pviv.o 
# OBJS2 = xps.cfg
# command to port program in working folder
PORT = sudo cp 
DIR = /usr/local/bin/
DIR2 = /usr/share/applications/

# CD
CD = cd

# program executable file name.
PROG = pviv
LAUNCHER = pviv.desktop

# top-level rule, to compile everything.
all: $(PROG)
	$(RM) $(OBJS) 

# rule to link the program
$(PROG): $(OBJS)
#	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)
#	$(PORT) $(PROG) $(DIR)
	$(CC) $(SOURCE) $(CFLAGS) -o $(PROG)

# rule for file "OBJS".
#$(OBJS): $(SOURCE)
#	$(CC) $(CFLAGS) -c $(SOURCE)

# rule for final build
build: $(PROG)
	$(RM) $(OBJS) 

# rule for cleaning re-compilable files.
clean:
	$(RM) $(PROG) $(OBJS)
#	$(RM) $(DIR)$(PROG)

# rule to port ipadd in the working folder
install:
	$(PORT) $(PROG) $(DIR)
	sudo chmod 755 $(PROG)
	$(PORT) $(LAUNCHER) $(DIR2)
	

# rule to port ipadd in the working folder
uninstall:
	sudo $(RM) $(DIR)$(PROG) 
	sudo $(RM) $(DIR2)$(LAUNCHER) 

