LIB =	fmt

SRCS =				\
	format		\
	posix

ifndef KBE_ROOT
export KBE_ROOT := $(subst /kbe/src/lib/dependencies/$(LIB),,$(CURDIR))
endif

include $(KBE_ROOT)/kbe/src/build/common.mak

# GCC on current Linux runners emits C++20 compatibility warnings for this
# vendored fmt version even when compiling as C++11. Keep Werror for the rest
# of the tree, but do not fail fmt on that forward-compat warning.
CXXFLAGS += -Wno-error=c++20-compat
