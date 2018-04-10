
# IAN


## Building

CMake >= 3.7 is required.


### Server

A compiler supporting C++14 is required to build the server.

Dependencies are managed through [CMake](http://cmake.org/) and [hunter](https://github.com/ruslo/hunter).

For Visual Studio, you might have to run CMake from the "Developer Command Prompt for VS" (hunter bug when building boost).

Dependencies (automated with hunter):

* [spdlog](https://github.com/gabime/spdlog)
* [boost](http://www.boost.org/) (asio, beast)
* [OpenSSL](https://www.openssl.org/)

Optional dependencies:

* [easy_profiler](https://github.com/yse/easy_profiler) (has to be built manually)


### Client

Client is built using [webpack](https://webpack.js.org/).

Dependencies (have to be installed):

* [npm](https://www.npmjs.com/)
* [yarn](https://yarnpkg.com/) (optional but recommended)



## License

The IAN engine is licensed under [Mozilla Public License 2.0](https://tldrlegal.com/license/mozilla-public-license-2.0-(mpl-2)).

IAN modules are subject to their own licenses.
