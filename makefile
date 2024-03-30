EXE=final

# Main target
all: $(EXE)

# Msys/MinGW
ifeq "$(OS)" "Windows_NT"
CFLG=-Ofast -fopenmp -Wall -DUSEGLEW
LIBS=-lfreeglut -lglew32 -lglu32 -lopengl32 -lm
CLEAN=del /Q *.exe *.o *.a
else
# OSX
ifeq "$(shell uname)" "Darwin"
RES=$(shell uname -r|sed -E 's/(.).*/\1/'|tr 12 21)
# Explicitly set the path to the Homebrew-installed LLVM compilers
CC=/opt/homebrew/opt/llvm/bin/clang
CXX=/opt/homebrew/opt/llvm/bin/clang++
CFLG=-Ofast -Wall -Wno-deprecated-declarations -DRES=$(RES) -fopenmp
# Add the necessary flags for Homebrew's LLVM
CFLG+= -I/opt/homebrew/opt/llvm/include
LDFLAGS+= -L/opt/homebrew/opt/llvm/lib -Wl,-rpath,/opt/homebrew/opt/llvm/lib
LIBS=-framework GLUT -framework OpenGL
# Linux/Unix/Solaris
else
CFLG=-Ofast -fopenmp -fopenacc -Wall -g
LIBS=-lglut -lGLU -lGL -lm
endif
# OSX/Linux/Unix/Solaris
CLEAN=rm -f $(EXE) *.o *.a
endif

# Dependencies
fatal.o: fatal.c CSCIx229.h
errcheck.o: errcheck.c CSCIx229.h
print.o: print.c CSCIx229.h
loadtexbmp.o: loadtexbmp.c CSCIx229.h
loadobj.o: loadobj.c CSCIx229.h
projection.o: projection.c CSCIx229.h

final.o: final.c CSCIx229.h stb_perlin.h global_config.h graphics_utils.h chunk.h biome.h
global_config.o: global_config.c global_config.h

# Create archive
CSCIx229.a: fatal.o errcheck.o print.o loadtexbmp.o loadobj.o projection.o
	ar -rcs $@ $^

# Compile rules
.c.o:
	$(CC) -c $(CFLG) $<
.cpp.o:
	$(CXX) -c $(CFLG) $<

# Link
final: final.o CSCIx229.a global_config.o
	$(CC) $(CFLG) -o $@ $^ $(LIBS)

# Clean
clean:
	$(CLEAN)
