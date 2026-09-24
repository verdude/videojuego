.PHONY: build debug reldebug release clean

build: debug reldebug release

debug:
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug

reldebug:
	cmake -S . -B build/reldebug -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cmake --build build/reldebug

release:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release

clean:
	rm -rf build
