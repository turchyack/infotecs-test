PROJECT_ROOT = ..
CXXFLAGS += -I $(PROJECT_ROOT)/include -Wall 
EXTRA_DEPS += Makefile $(PROJECT_ROOT)/common.mk $(wildcard $(PROJECT_ROOT)/include/*.hpp)