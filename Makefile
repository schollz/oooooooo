
builder: build
	cd build && cmake --build . --config Release -- -j$(nproc)

build:
	mkdir -p build
	cd build && cmake -DCMAKE_BUILD_TYPE=Release ..

run: builder
	./build/clients/softcut_jack_osc/softcut_jack_osc

clean:
	rm -rf build