CC:=clang										# compiler
# CFLAGS+=-O2									  # compiler flags, this is optimization level 2
LDFLAGS+=-lpthread
LDFLAGS+=-lrt
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))		# search for all .c files. patsubst creates .o files with same names

TARGET=NucTer

bin: $(TARGET)									# first/default target

$(OBJS): %.o: %.c								# dereference of OBJS variable; creates .o files for every .c file
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@		# call compiler for every input with all C/CPP flags and create outptut
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:											# remove all unnecessary files
	$(RM) $(OBJS) $(TARGET)						# calls rm on command line and gets rid of all objects and target