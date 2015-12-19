
# IAN

IAN is a multiplayer game engine in early development.
The client is web-based, and written in full Javascript, supported by HTML and CSS.
The server is written in C++, using Javascript for game logic.


## Dependencies

The most important tool used in IAN is [Premake](https://premake.github.io) (5.0 alpha7).
Unless you want to work on the server's C++ code, this is the only tool that you will need.

To compile the server, you will need the following libraries:

* [Mozilla's SpiderMonkey](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey) (38)
* [ASIO](http://think-async.com/Asio/) (1.11.0)
* [Websocket++](http://www.zaphoyd.com/websocketpp/) (0.6.1)
* [Optional] [SFML](http://www.sfml-dev.org) (2.3.2)

The project also uses the following tools, but you do not have to download them manually:

* [Closure Compiler](https://developers.google.com/closure/compiler/)
* [YUI Compressor](https://yui.github.io/yuicompressor/)


## Project folders

### game

Contains game code and resources.
The deploy script identifies the different files by name:

client.html is the template for the client HTML page.

A file or folder starting with "rc." will be deployed as a client resource.
For example "my-game/rc.img/tileset.png" will be copied as "img/tileset.png" in the client files.

Game files can be Javascript files (.js) or CSS files (.css).
A file or folder starting with "cl." will be considered as client-only. CSS files are always client-only.
On the other hand, a file or folder starting with "sv." will be considered as server-only.

All the files in a folder will always be executed before the files in the sub-folders.
Finally you can add a number "123." at the very beginning of your files and folders to change the execution order. The lowest number ("0.") has the highest priority.
For example "1.first.js" will be executed before "2.second.js". Both files will be executed before "third.js", a file without a number in the same folder.

Priority and client / server tags can be combined, priority first ("1.cl.hello.lua").


### src

Contains server's C++ code.


### scripts

Contains all the Lua scripts used by Premake.


## License

The code is licensed under AGPLv3 for now (see LICENSE file).
I am also considering the MIT license, which is less restrictive.
