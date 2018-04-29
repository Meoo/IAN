
# IAN


## Server

A compiler supporting C++14 is required.

CMake >= 3.7 is required.

Dependencies:

* [spdlog](https://github.com/gabime/spdlog)
* [boost](http://www.boost.org/) (asio, beast, property_tree)
* [OpenSSL](https://www.openssl.org/)

Optional dependencies:

* [easy_profiler](https://github.com/yse/easy_profiler)


### Hunter

Dependencies can be managed through [CMake](http://cmake.org/) and [hunter](https://github.com/ruslo/hunter).

Use `IAN_HUNTER=ON` when configuring the CMake project.


### vcpkg

If you are using [vcpkg](https://github.com/Microsoft/vcpkg), install the dependencies:

`vcpkg install spdlog openssl boost-asio boost-beast boost-property-tree`

Then, configure with CMake in your build directory:

`cmake [...]path/to/IAN -DCMAKE_TOOLCHAIN_FILE=[...]vcpkg\scripts\buildsystems\vcpkg.cmake`



## Client

Client is built using [webpack](https://webpack.js.org/).

Dependencies (have to be installed):

* [npm](https://www.npmjs.com/)
* [yarn](https://yarnpkg.com/) (optional but recommended)



# License

The IAN engine is licensed under [Mozilla Public License 2.0](https://tldrlegal.com/license/mozilla-public-license-2.0-(mpl-2)).

IAN modules are subject to their own licenses.
