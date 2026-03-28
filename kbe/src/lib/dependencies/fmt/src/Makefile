LIB =	fmt

SRCS =				\
	format		\
	os

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/$(LIB)/src,,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

# The vendored fmt build still compiles as part of the legacy Unix static-lib
# pipeline. Disable forward-compat warnings that would otherwise trip -Werror.
CXXFLAGS += -Wno-error=c++20-compat
