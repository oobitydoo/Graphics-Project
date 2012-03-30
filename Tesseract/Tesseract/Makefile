CC = g++
CFLAGS = -Wall 
PROG = tesseract 

SRCS = Tesseract.cpp InitShader.cpp

LIBS = -lglut -lGLU -lGL -lGLEW

all: $(PROG)

$(PROG):	$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)

clean:
	rm -f $(PROG)
