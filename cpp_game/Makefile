.PHONY: build debug reldebug release clean

reldebug:
	cmake -S . -B build/reldebug -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cmake --build build/reldebug

debug:
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug

release:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release

clean:
	-make -C build/release clean
	-make -C build/debug clean
	-make -C build/reldebug clean

all: debug reldebug release

