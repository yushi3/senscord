# Overview

This section describes how to use CMake and how to build the SensCord.

## Preparation of build environment

### Install CMake

Use CMake for the build.
Please download the file from the following URL.
URL: https://cmake.org/download/

cmake-gui.exe under the bin folder that is created by decompressing the compressed file is for making the Makefile or the solution file.

Available versions differ depending on the OS.
- Linux : Version2.8 or later
- Windows : Version3.4 or later

### Install Visual Studio

Since Visual Studio is used for building, download the installer from the Visual Studio site, and install it.
You can also build in the Visual Studio Community.
Install C++ development tools at the same time.

### Install Python

If you want to use Python API, please install Python.
Version supports 2.7 or later.

## Available CMake options

When starting the CMake configure, a list of variables as follows is displayed on the screen and values can be changed.

- CMAKE_CONFIGURATION_TYPES  
A list of selectable build types. Do not change this.

- CMAKE_INSTALL_PREFIX  
Specify the installation destination of the built files (xxx.dll, xxx.lib.. ).

- SENSCORD_LOG_ENABLED  
When set to ON, the SensCord library output the log.

- SENSCORD_LOG_OSAL_ENABLED  
When set to ON, the SensCord osal library output the log.

- SENSCORD_LOG_TYPE  
Set the log output destination.
In Linux, CONSOLE / SYSLOG / FILE can be selected, and in Windows, CONSOLE / FILE can be selected.
For the option, enter the output destination as a character string.

  Example:
  ```
  cmake ../ -DSENSCORD_LOG_TYPE = CONSOLE
  ```
  **For the CMake GUI, enter directly in the text box.**

- SENSCORD_LOG_TIME_ENABLED  
When set to ON, the time is added to the log output.

- SENSCORD_STATUS_TRACE_ENABLED  
Set the status class trace function.
When set to ON, function call traces are added to the Status class error log.

- SENSCORD_API_C  
Set C API build ON / OFF.
When set to ON, the SensCord C API is an available.

- SENSCORD_API_PYTHON  
Enable the python API build.
When set to ON, the SensCord python API is an available.

- SENSCORD_SAMPLE  
Set ON / OFF of build of sample application.
When set to ON, the sample application in the SensCord is built with the library.

- SENSCORD_COMPONENT_PSEUDO  
Sets ON / OFF of build of the pseudo image component.
The pseudo image component is a component that outputs dummy data.

# Build procedure (CMakeGUI on Windows)

## 1. Launch cmake-gui
Please execute cmake-gui.exe.

## 2. Specify project directory  
Click "Browse Source" button of "Where is the source code" and
select the top directory of the repository.
(The directory where CMakeLists.txt is located).

## 3. Select build directory  
Specify the directory for CMake products and Visual Studio solution files.
By default, the same directory as CMakeLists.txt is set, but it is possible to select a folder by pressing the "Browse build" button of "Where to build the binaries".
Create a "build" directory directly under the SensCord repository.

## 4. Run Configure  
Read CMakeLists.txt and create a cache file.
When you press the button, the Visual Studio compiler selection dialog will appear. Specify the Visual Studio (The version you have) compiler.
The compiler differs between the 32-bit version and the 64-bit version.

When the compiler is specified, CMakeLists.txt is analyzed.
When "Configuring done" is displayed, the configure operation is complete.

If you get an OpenCV reference error when building the sample app, set the OpenCV installation directory as an environment variable.

## 5. Project output  
Click the Generate button.
When "Generating done" is displayed, the Visual Studio solution file (.sln file) is output to the build directory.

## 6. Executing the build  
Start Visual Studio by executing the solution file output to the build directory.
The project "ALL_BUILD" is displayed under "cmake-common" in the solution explorer, so build this project.
The entire build is done in the project corresponding to make all.

## 7. Building the sample  
Return to "5. Project output", and if "SENSCORD_SAMPLE" is checked in the CMake option, the sample application will be built.

## 8. Run the sample app  
When the build is complete, SimpleStreamPlayer.bat is placed under build/output.
When this bat file is executed, SimpleStreamPlayer.exe is executed on the console and the program execution log is displayed.
Even if SimpleStreamPlayer is modified and build on Visual Studio, it can be executed with bat.

## 9. Delete unnecessary projects  
This procedure is for deleting a project or starting over from configure.
To delete the cache file, select "File-> delete cache" from the cmake-gui menu.
Then delete all the contents of the build directory.


# Build procedure (Linux console)

## 1. Preparing the build directory
Create a build directory under the senscord directory.
Change directory to the build directory. (# cd build)

## 2. Run CMake
CMake can be executed with the following command.
```
cmake ../
```

## 3. How to specify options  
The option value can be changed by prefixing the option name with -D.
The values that can be set vary depending on the type of option.
  * STRING: String (Example: Debug)
  * PATH: File path (Example: /home/hoge)
  * BOOL: Boolean value (ON / OFF)

The following is an example of command.
Example: If you want to enable pseudo build
```
cmake ../ -DSENSCORD_COMPONENT_PSEUDO = ON
```

## 4. How to confirm option selection
You can check the option selection status (cache value) by adding an argument to the CMake command.
Example:
```
cmake ../ -L [A | H]
```
- A: also displays the cache value common to CMake
- H: Show cache variables defined in CMakeLists.txt with comments

## 5. Make
When CMake ends normally, a Makefile is output to the build directory.
You can build by executing make in the build directory.

## 6. Using shell
The following shells are prepared in the scripts folder so that the CMake configuration can be created with preset settings.

- cmake_debug_buld_enable.sh  
Script to enable debug build.
Debug build (-g -O0) can be done by using this script.

- cmake_sample_enable.sh  
This is a script to enable build of sample application.
If this script is used, the application below sample will be built.

- cmake_component_image_pseudo.sh  
Script to enable the pseudo Component build.
Using this script will turn off other TARGET builds.

# Build procedure (Android NDK on Linux console)

## 1. Prepare Android NDK (Linux)

例）インストール場所: `~/android/ndk`

```shell-session
$ mkdir ~/android && cd ~/android
$ wget https://dl.google.com/android/repository/android-ndk-r23b-linux.zip
$ unzip android-ndk-r23b-linux.zip
$ ln -s ~/android/android-ndk-r23b ~/android/ndk
```

最新パッケージは以下のサイトを確認してください。  
<https://developer.android.com/ndk/downloads>

## 2. Build SensCord (target: android-armeabi-v7a)

```shell-session
$ cd <senscord-repository>
$ mkdir build && cd build

$ cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=~/android/ndk/build/cmake/android.toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=android-24 \
    -DCMAKE_INSTALL_PREFIX=./export

$ make
$ make install
install to <senscord-repository>/build/export
```

以下のオプションはターゲットに従って変更します。  

```shell-session
-DANDROID_ABI=armeabi-v7a \
-DANDROID_PLATFORM=android-24 \
```

詳細は以下のサイトを確認してください。  
<https://developer.android.com/ndk/guides/cmake>


# Use of build target

There are build targets that can be used to delete unnecessary files and change CMake options in builds using the make command and Visual Studio.
In the make command, use the build target name as the argument to make, and in the case of Visual Studio, build the project using MSBuild or Solution Explorer.

- clean_all (Linux only)  
Delete build results and output folder at the same time.

- clean_output  
Delete the output directory that stores the build results.

- option_all_enabled  
Turn on all available CMake options.

- option_all_disabled  
Turn off all available CMake options.

# File export procedure

By executing make install, the header file and the file generated by the build can be exported to any directory.
On Windows without make command, you can export by building Visual Studio INSTALL project.

## How to specify the export destination
Specify the export directory in the CMake variable CMAKE_INSTALL_PREFIX.
If it is CMakeGUI, it can be changed by operation on the GUI.
For Linux, specify the directory with the -D option when executing CMake.
Example:
```
cmake -DCMAKE_INSTALL_PREFIX = /home/xxx
```

## execute export

### Linux
Files are installed in the specified directory by typing make install on the CMake build directory.

### Windows
Select INSTALL from the Visual Studio project and execute the build to install the files in the specified directory.

## Files output by export
Details of directories and files exported by make install.

- bin  
If SENSCORD_SAMPLE is ON, the sample application will be stored.

- include/senscord  
Contains public headers used by applications and components.

- include/senscord/c_api  
Contains the header used when SensCord is used in C language.
Exported when SENSCORD_API_C = ON.

- include/senscord/develop  
Contains the headers required to develop a component.
When developing a component using the exported file, it is necessary to refer to include/senscord and include/senscord/develop.

- lib  
Stores the SensCord library.

- lib/cmake/senscord  
The SensCord module that can be used with CMake is installed.

- lib/senscord  
OS Abstract Layer library is stored

- lib/senscord/allocator  
Stores the allocator library.

- lib/senscord/component  
Stores the library and xml file of the component to be built.

- share/senscord/config  
Stores the config file used by SensCord.


# Procedure to build by referring to exported SensCord

This is the procedure to build an application or component using SensCord.

## About find_package
CMake has a function called find_package that searches for installed files using a Makefile created by CMake.
By using find_package, you can use SensCord libraries and headers without include_directories or link_directories.

## How to use find_package
When searching SensCord with find_package, CMakeLists.txt is described as follows.
Example of use:
> find_package (senscord REQUIRE)

By setting REQUIRE as the second argument, CMake will exit with an error on the spot if the exported file is not found.
If the file is found, SensCord will be available in target_link_libraries.

Example of use:  
> target_link_libraries (app or library name senscord)

## Set search path
When using find_package, it is necessary to set the path to search for SensCord packages.
The setting method differs depending on the environment where the build is executed.

### Linux environment
If the installation location of SensCord is less than /usr or /usr/local, there is no need to set a search path.
If you installed in a directory other than the above, you must use the CMAKE_PREFIX_PATH option when running CMake to set the SensCord installation path.

If installed in /home/xxxxxx/senscord:
```
cmake ../ -DCMAKE_PREFIX_PATH = /home/xxxxxx/senscord
```

### Windows environment
If SensCord is installed under C:\Program Files, you do not need to set a search path.
If you installed in a directory other than the above, you must use the CMAKE_PREFIX_PATH option when running CMake to set the SensCord installation path.

To use CMAKE_PREFIX_PATH with CMakeGUI, press the + Add Entry button on the screen and in the dialog box that appears, set CMAKE_PREFIX_PATH for name, FILEPATH for Type, and the directory name of the installation destination for Value.

When installed in C:\work\senscord:
Set CMAKE_PREFIX_PATH definition and the following values in AddEntrty
  - Name = CMAKE_PREFIX_PATH
  - Type = FILEPATH
  - Value = C:\work\senscord

### Run build
If you run CMake and the command passes, you can build as it is.
If an error occurs in find_package, review CMAKE_PREFIX_PATH.
Alternatively, you can build by setting the path of the directory where the following files are placed in the senscord_DIR option.

  - senscord-config.cmake
  - senscord-config-debug.cmake
