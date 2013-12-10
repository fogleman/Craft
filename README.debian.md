## Debian 7 Installation

If you have GLEW already installed in system, you will probably get
linker error like:

```bash
...
undefined reference to __glewDeleteBuffers
...
```
because of old library version in our lovely distro.
In that case you are welcome to build new version of GLEW by youself:

[http://glew.sourceforge.net/index.html](http://glew.sourceforge.net/index.html)

After you successfully build GLEW,  you must properly set the ```GLEW_LOCATION``` variable in CMakeLists.txt
or in your shell environment. That's all. Now you can:

```bash
cmake .
make
```

