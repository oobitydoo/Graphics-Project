CC = g++
CFLAGS = -Wall 
PROG = project2 

SRCS = Project2.cpp InitShader.cpp

LIBS = -lglut -lGLU -lGL -lGLEW

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
