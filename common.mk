PROJECT_ROOT = ..
CXXFLAGS += -std=c++20 -Wall -Wextra -Wpedantic -Werror -I $(PROJECT_ROOT)/include
EXTRA_DEPS += Makefile $(PROJECT_ROOT)/common.mk $(wildcard $(PROJECT_ROOT)/include/*.hpp)
