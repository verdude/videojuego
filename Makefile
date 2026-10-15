CMAKE_FLAGS :=
ifeq ($(OS),Windows_NT)
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		CMAKE_FLAGS += -DCMAKE_PREFIX_PATH="$(shell brew --prefix)"
	endif
endif

.PHONY: build debug reldebug release clean

reldebug:
	cmake -S . -B build/reldebug -DCMAKE_BUILD_TYPE=RelWithDebInfo $(CMAKE_FLAGS)
	cmake --build build/reldebug

debug:
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug $(CMAKE_FLAGS)
	cmake --build build/debug

release:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release $(CMAKE_FLAGS)
	cmake --build build/release

clean:
	-make -C build/release clean
	-make -C build/debug clean
	-make -C build/reldebug clean

all: debug reldebug release

