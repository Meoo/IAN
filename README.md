
# IAN

IAN is a multiplayer game engine in early development.
The client is web-based, and written in full Javascript, supported by HTML and CSS.
The server is written in C++, using Javascript for game logic.

The server will be supported for Windows, Linux and maybe MacOSX.


## Dependencies 

The most important tool used in IAN is [Premake](https://premake.github.io) (5.0 alpha7).

Premake is enough to deploy IAN's client, but if you want to work with C++ code you will have to install aditionnal tools.

To compile C++ modules targeting Javascript, you must have:

* [Emscripten](https://kripken.github.io/emscripten-site/)

To compile the server, you will need to get the following libraries:

* [Mozilla's SpiderMonkey](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey) (38)
* [ASIO](http://think-async.com/Asio/) (1.11.0)
* [Websocket++](http://www.zaphoyd.com/websocketpp/) (0.7.0)
* [OpenSSL](https://www.openssl.org/) (1.0.1q)
* [Optional] [SFML](http://www.sfml-dev.org/) (2.3.2)

For Windows, you also need VS2013 or VS2015. MinGW is not supported because it can't build SpiderMonkey.

The client uses the following Javascript libraries:

* [jQuery](https://jquery.com/) (2.1.4)
* [Font Awesome](https://fortawesome.github.io/Font-Awesome/) (4.5.0)

The project also uses the following tools internally:

* [Closure Compiler](https://developers.google.com/closure/compiler/)
* [YUI Compressor](https://yui.github.io/yuicompressor/) (2.4.8)


## Project folders


### source

Contains all the code.

Each folder in "source" is a package. Each package contains a package.lua file which is loaded by the build system.

Each package can contain Javascript code and C++ code. The C++ code can be compiled to a native server module or to Javascript using Emscripten.

Packages are sorted and loaded by alphanumerical order (numbers first, then upper-case, then lower-case).

The two default packages are :

* 000-ian-shell : IAN's client shell.

* 100-ian-core :  IAN's client libraries and server code.


### scripts

Contains all the Lua scripts used by Premake.


## License

The code is licensed under Mozilla Public License 2.0 (see LICENSE file).
