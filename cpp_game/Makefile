build/reldebug/game:
	cmake -S . -B build/reldebug -DCMAKE_BUILD_TYPE=RelWithDebInfo
	cmake --build build/reldebug

build/debug/game:
	cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	cmake --build build/debug

build/release/game:
	cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	cmake --build build/release

.PHONY: build clean
build: build/debug/game build/reldebug/game build/release/game
clean:
	rm -rf build
