include ../common.mk
SRCS = main.cpp
all: server


server: $(SRCS) $(EXTRA_DEPS)
	$(CXX) -o server $(CXXFLAGS) $(SRCS) 
	
clean:
	rm -rf *.o server
	