# Luna
Luna is a statically linked library which provides a layer of abstraction over the Vulkan API with the goal of keeping the same performance of using Vulkan while reducing the amount of boilerplate code which is needed to create a simple application.

While it is still in a very early state right now, it is actively being developed during my spare time.

## Versioning
I follow sematic versioning for my version numbers, and until 1.0.0 an increment in the minor version will be used to indicate an API-breaking change, while changes to the patch version are non-breaking.

## Bindings
Currently Luna only has bindings for C11, though in the future I hope to not only provide C++ bindings but also to support older versions of the C specification.

## Usage
As I've been more focused on getting the code working, there is no real documentation available yet. I do plan to document the entire library in full at some point, but as of right now it does not have any.
### Examples
There are two simple examples located in the project itself which both use C11 and SDL3. These will automatically be compiled when compiling Luna as a top-level project, but will not be compiled if Luna is compiled as a dependency of your own application. In addition to these examples, you can find an example of it being used in a more complex project by looking at https://github.com/droc101/c-game-engine

## Building
Luna uses CMake to automatically fetch dependencies, meaning that simply configuring the CMake project should set everything up to compile first try. Additional options for configuring the build process can be found in the `cmake/options.cmake` file.
