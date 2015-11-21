# This Makefile is used by travis to trigger the different build steps
# vim: ts=4 sts=4 sw=4 noexpandtab

SHELL=/bin/bash
PROJECT_DIR=$$PWD/..

linux: build/konstructs
tar: konstructs-client.tar.bz2


build/konstructs:
	@echo -e "\033[1m"
	@echo "# Use Docker to build a Linux build"
	@echo -e "\033[0m"

	docker run -ti -v ${PROJECT_DIR}:/build \
		-e HOST_UID=$$UID \
		nsgb/konstructs-build-image-linux \
		make docker-linux -C package

osx:
	@echo -e "\033[1m"
	@echo "# Build on OSX"
	@echo -e "\033[0m"

	brew update
	cd .. && cmake . && make

docs:
	@echo -e "\033[1m"
	@echo "# Use Docker to build docs with Doxygen"
	@echo -e "\033[0m"

	docker run -ti -v ${PROJECT_DIR}:/build \
		-e HOST_UID=$$UID \
		nsgb/konstructs-build-image-docs \
		doxygen

/usr/local/bin/bin2c:
	@echo -e "\033[1m"
	@echo "# Build a Linux version of bin2c for the cross compile"
	@echo -e "\033[0m"

	gcc -o bin2c ../dependencies/nanogui/resources/bin2c.c
	mv bin2c /usr/local/bin/bin2c

windows:
	@echo -e "\033[1m"
	@echo "# Use Docker to cross compile a Windows build"
	@echo -e "\033[0m"

	docker run -ti -v ${PROJECT_DIR}:/build \
		-e HOST_UID=$$UID \
		nsgb/konstructs-build-image-windows \
		make docker-windows -C package

/tmp/build-deps/zlib:
	@echo -e "\033[1m"
	@echo "# Download zlib lib for Windows"
	@echo -e "\033[0m"

	mkdir -p /tmp/build-deps/zlib
	wget http://netcologne.dl.sourceforge.net/project/gnuwin32/zlib/1.2.3/zlib-1.2.3-lib.zip \
		-O /tmp/build-deps/zlib-1.2.3-lib.zip
	echo "b0a2df1a2b0dd0bd801d71d06ce42360  /tmp/build-deps/zlib-1.2.3-lib.zip" | md5sum -c -
	cd /tmp/build-deps/zlib && unzip ../zlib*lib.zip

/tmp/build-deps/zlib-bin:
	@echo -e "\033[1m"
	@echo "# Download zlib bin for Windows"
	@echo -e "\033[0m"

	mkdir -p /tmp/build-deps/zlib-bin
	wget http://netcologne.dl.sourceforge.net/project/gnuwin32/zlib/1.2.3/zlib-1.2.3-bin.zip \
		-O /tmp/build-deps/zlib-1.2.3-bin.zip
	echo "0b431b557399c1b3948c13c803a22c95  /tmp/build-deps/zlib-1.2.3-bin.zip" | md5sum -c -
	cd /tmp/build-deps/zlib-bin && unzip ../zlib*bin.zip

docker-windows: export CC = /usr/bin/x86_64-w64-mingw32-gcc
docker-windows: export CXX = /usr/bin/x86_64-w64-mingw32-g++
docker-windows: export CPP = /usr/bin/x86_64-w64-mingw32-cpp
docker-windows: export RANLIB = /usr/bin/x86_64-w64-mingw32-ranlib
docker-windows: /tmp/build-deps/zlib /usr/local/bin/bin2c
	@echo -e "\033[1m"
	@echo "# Build the Windows build inside Docker"
	@echo -e "\033[0m"

	mkdir -p build-windows
	cd build-windows && \
		cmake -DCMAKE_TOOLCHAIN_FILE=./Toolchain-mingw32.cmake ../..
	cd build-windows && make
	chown -R $$HOST_UID build-windows
	

docker-linux:
	@echo -e "\033[1m"
	@echo "# Build the Linux build inside Docker"
	@echo -e "\033[0m"

	mkdir -p build
	cd build && cmake ../.. && make
	chown -R $$HOST_UID build

konstructs-client.tar.bz2: linux
	@echo -e "\033[1m"
	@echo "# Package the build in a tar-ball"
	@echo -e "\033[0m"

	mkdir -p konstructs-client-linux
	cp build/konstructs konstructs-client-linux
	cp -r ../shaders ../textures konstructs-client-linux
	tar cfj konstructs-client.tar.bz2 konstructs-client-linux
	rm -rf konstructs-client-linux

package-prep:
	@echo -e "\033[1m"
	@echo "# Fetch remote tags and unshallow the repo (if needed)"
	@echo -e "\033[0m"

	# Fetch the remote tags
	git fetch --tags

	# Travis only fetch the 50 last commits, this fixes that.
	git fetch --unshallow || :

deb: linux package-prep
	@echo -e "\033[1m"
	@echo "# Use Docker to make the deb package"
	@echo -e "\033[0m"

	docker run -ti -v ${PROJECT_DIR}:/build \
		-e TRAVIS_BUILD_NUMBER \
		-e HOST_UID=$$UID \
		nsgb/konstructs-build-image-linux \
		make docker-deb -C package

rpm: linux package-prep
	@echo -e "\033[1m"
	@echo "# Use Docker to make the rpm package"
	@echo -e "\033[0m"

	docker run -ti -v ${PROJECT_DIR}:/build \
		-e TRAVIS_BUILD_NUMBER \
		-e HOST_UID=$$UID \
		nsgb/konstructs-build-image-linux \
		make docker-rpm -C package

zip: windows
	@echo -e "\033[1m"
	@echo "# Use Docker to make the zip package"
	@echo -e "\033[0m"

	docker run -ti -v ${PROJECT_DIR}:/build \
		-e TRAVIS_BUILD_NUMBER \
		-e HOST_UID=$$UID \
		nsgb/konstructs-build-image-windows \
		make docker-zip -C package


docker-package-prep:
	@echo -e "\033[1m"
	@echo "# Move files in place"
	@echo -e "\033[0m"

	mkdir -p konstructs-client-package/usr/local/bin/
	mkdir -p konstructs-client-package/usr/local/share/konstructs-client
	mkdir -p konstructs-client-package/usr/share/applications
	cp -r ../shaders konstructs-client-package/usr/local/share/konstructs-client
	cp -r ../textures konstructs-client-package/usr/local/share/konstructs-client
	cp build/konstructs konstructs-client-package/usr/local/bin/konstructs-client
	cp konstructs-client.desktop \
		konstructs-client-package/usr/share/applications/konstructs-client.desktop

docker-deb: docker-package-prep
	@echo -e "\033[1m"
	@echo "# Make a deb package"
	@echo -e "\033[0m"

	fpm -s dir -t deb \
		-C konstructs-client-package \
		-v $(shell git describe --abbrev=0 --tags | tr -d v) \
		--iteration $${TRAVIS_BUILD_NUMBER:=0} \
		--license MIT \
		--depends libglfw2 \
		--depends zlib1g \
		--maintainer "Stefan Berggren <nsg@nsg.cc>" \
		--url https://github.com/konstructs/client \
		-n konstructs-client \
		usr/local/bin/konstructs-client \
		usr/local/share/konstructs-client/textures \
		usr/local/share/konstructs-client/shaders \
		usr/share/applications/konstructs-client.desktop

	chown $$HOST_UID *.deb
	rm -rf konstructs-client-package

docker-rpm: docker-package-prep
	@echo -e "\033[1m"
	@echo "# Make a rpm package"
	@echo -e "\033[0m"

	fpm -s dir -t rpm \
		-C konstructs-client-package \
		-v $(shell git describe --abbrev=0 --tags | tr -d v) \
		--iteration $${TRAVIS_BUILD_NUMBER:=0} \
		--license MIT \
		--depends zlib \
		--maintainer "Stefan Berggren <nsg@nsg.cc>" \
		--url https://github.com/konstructs/client \
		-n konstructs-client \
		usr/local/bin/konstructs-client \
		usr/local/share/konstructs-client/textures \
		usr/local/share/konstructs-client/shaders \
		usr/share/applications/konstructs-client.desktop

	chown $$HOST_UID *.rpm
	rm -rf konstructs-client-package

docker-zip: /tmp/build-deps/zlib-bin
	@echo -e "\033[1m"
	@echo "# Make a zip archive"
	@echo -e "\033[0m"

	mkdir -p konstructs-client
	cp build-windows/konstructs.exe konstructs-client/konstructs-client.exe
	cp /tmp/build-deps/zlib-bin/bin/zlib1.dll konstructs-client
	cp -r ../shaders ../textures konstructs-client
	zip -r konstructs-client.zip konstructs-client
	chown $$HOST_UID *.zip
	rm -rf konstructs-client

upload-tar: export MAKE_TAR = true
upload-tar:
	./bintray

upload-deb: export MAKE_DEB = true
upload-deb:
	./bintray

upload-zip: export MAKE_ZIP = true
upload-zip:
	./bintray

clean:
	rm -rf \
		konstructs-client-linux \
		build build-windows \
		konstructs-client.tar.bz2 \
		konstructs-client_*_amd64.deb \
		konstructs-client-*.x86_64.rpm \
		konstructs-client.zip

.PHONY: build-linux build-osx build-window nop clean
.PHONY: docker-linux
.PHONY: package-tar package-deb package-rpm package-linux package-all
