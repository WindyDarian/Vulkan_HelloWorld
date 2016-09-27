# Vulkan_HelloWorld

Just a plain old Vulkan project following https://vulkan-tutorial.com/ by Overv

Not that strictly following though...

Through the tutorial, I have learned __how to create a basic vulkan graphics pipeline__.

![ran on Ubuntu](/Screenshots/screenshot_ubuntu.png)

### build

Use CMake to build the program.

#### Windows
Make sure you have Vulkan SDK and Visual Studio 2015 or up, then:
```
mkdir build
cd build
cmake-gui ..
```
And `Configure`(select `"Visual Studio 2015 x64"`), `Generate`, then you have Visual Studio project files.

Remember to copy `src/content` to `build` folder in order to run it.


#### Linux
Make sure `VULKAN_SDK` is set to `x86_64` folder under Vulkan SDK path and you have `LD_LIBRARY_PATH` and `VK_LAYER_PATH` set by running `source ./setup-env.sh` at Vulkan SDK folder, and then
```
mkdir build
cd build
cmake ..
make
```
copy `src/content` to `build` folder, then run the executable

### Screenshots

![typical triangle](/Screenshots/1.JPG)

![texture mapping and depth test](/Screenshots/depth_test.JPG)

![loading model](/Screenshots/loading_model.JPG)

![ran on Ubuntu](/Screenshots/screenshot_ubuntu.png)
