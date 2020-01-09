TARGET=test
CC=g++
PCH_HEADER=pch.h
PCH=pch.h.gch
OBJ=main.o yaml_test.o json_test.o config_test.o
CFLAGS=-g -DNDEBUG -O3 -I ~/json-3.7.3/single_include -I/usr/include -I/usr/local/include 
CXXFLAGS=-I../include -std=c++14
LDFLAGS= -L/usr/lib -L/usr/local/lib -lyaml-cpp -lconfig++

all : $(TARGET)

-include $(OBJS:.o=.d) 

$(PCH) : $(PCH_HEADER)
	$(CC) $(CFLAGS) $(CXXFLAGS) -x c++-header -o $@ $<
	
%.d: %.cpp  
	@set -e; rm -f $@; $(CC) -MM $< $(CFLAGS) $(CXXFLAGS) > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.o : %.cpp $(PCH)
	$(CC) -c $(CFLAGS) $(CXXFLAGS) -o $@ $< 

$(TARGET) : $(OBJ)
	libtool --tag=CXX --mode=link $(CC) $(LDFLAGS) -o $@ $^

clean:
	rm $(TARGET) $(OBJ) -f
