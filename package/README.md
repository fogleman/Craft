# Build it like we do at Travis

This file has a `Makefile` that we use to build the project. It uses normal make dependencies to resolve the needed steps. This requires Docker and is primary used on Travis, if you like to build a local build see BUILD.md at the repository root.

## Build a Linux build

This step uses Docker to start a Ubuntu 15.10 container to build the project. 

```
make linux
```

## Build a Windows build
This produces a `.zip` file containing everything you need to run the game under Windows. The files are produces from MinGW and are cross compiled from a Fedora 23 container.

```
make windows
```

## Build a OSX build
This will plain and simple build the project on OS X, no install packages are generated at the moment.

```
make osx
```

## Build a tar-ball
This build produces a `tar.bz2` with everything you need to run the game. This build step is primary made for our Travis builds that we [distribute at Bintray](https://bintray.com/konstructs/linux/client/view).

```
make tar
```

## Build a zip archive
This build produces a `.zip` with everything you need to run the game under Windows. This build step is primary made for our Travis builds that we [distribute at Bintray](https://bintray.com/konstructs/windows/client/view).

```
make zip
```

## Build Linux install packages
This build produces a `.deb` or a `.rpm`. We uses this to build packages on Travis for Bintray.

```
make deb
```

or

```
make rpm
```
