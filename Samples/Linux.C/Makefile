# define target filename
TARGETLIB = axtrace

# compiler common define
CC 				= g++
CPPFLAGS 	= -g -O2 -Wunused-result

#All object files
OBJS =\
	axtrace.linux.o \
	test.o

all : $(OBJS)
	$(CC) -o $(TARGETLIB) $^
	
axtrace.linux.o : ./axtrace.linux.c
	$(CC) $(CPPFLAGS) -c $< -o $@
	
%.o : ./%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f $(TARGETLIB)

