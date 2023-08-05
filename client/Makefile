include ../common.mk
SRCS = main.cpp
all: client

client: $(SRCS) $(EXTRA_DEPS)
	$(CXX) -o client $(CXXFLAGS) $(SRCS)

clean:
	rm -rf *.o client