# Cis1 webui server

> Continuous integration system webui server written in C++

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

This repository contains a webserver for the cis1 continuous integration system.
The server part is written in C++.
This component follows the rules described in the
[cis1-docs](https://github.com/tomsksoft-llc/cis1-docs/ "cis1 documentation") repository.
It's compatible with OS Linux; the support for OS Windows is not guaranteed, but we are working to add it.

## Description

This webserver is a part of the cis1 system.
It allows remote control of the core and adds additional functionality, such as:
1. Authentication, authorization, user management
2. Projects access rights
3. Ability to use WebHooks
4. Automation of some routine actions

All interoperations between the frontend and the web server, except for file operations, are carried out via WebSocket.

The default WebSocket address is:

```
ws://${SERVER_IP_ADDRESS}:${SERVER_PORT}/ws
```

The protocol used over the WebSocket is described on
[this](https://github.com/tomsksoft-llc/cis1-webui-native-srv-cpp/wiki/Protocol-description "Protocol description")
wiki page.

## Usage

Copy the executable into any directory.

Modify config_example.ini as you need to.

```ini
[global]
working_dir=${TEMP_FILES_DIRECTORY}
[http]
ip=${SERVER_IP_ADDRESS}
port=${SERVER_PORT}
doc_root=${STATIC_FILES_DIRECTORY}
[cis]
cis_root=${CIS_DIRECTORY}
ip=${INTERNAL_CORE_COMMUNICATION_IP}
port=${INTERNAL_CORE_COMMUNICATION_IP}
[db]
db_root=${DB_DIRECTORY}
```

Absolute paths are preferable.

Run

```console
$ ./cis1_srv ${PATH_TO_CONFIG}/config_example.ini
```

After the previous steps are completed, the web server will be running on the configured IP-address and port.
There is a reference implementation of the frontend for this web server, it is located in
[this](https://github.com/tomsksoft-llc/cis1-webui-native-front-vanilla "cis1 webui front vanilla")
repository. 
You can download it into the `www` directory, and change the WebSocket address within the frontend configuration.

The WebHooks usage tutorial can be found on
[this](https://github.com/tomsksoft-llc/cis1-webui-native-srv-cpp/wiki/Webhooks-tutorial "Webhooks tutorial")
wiki page.

## Compiling

### Linux

Build requirements:
> Conan, gcc 8.3, CMake 3.9+

Add the `tomsksoft` and `bincrafters` repositories to conan

```console
$ conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
$ conan remote add tomsksoft https://api.bintray.com/conan/tomsksoft/cis1
```

Switch to the build directory and install build dependencies

```console
$ conan install ${PATH_TO_SRC} --profile -s build_type=Release --build=missing
```

Generate build scripts with CMake

```console
$ cmake ${PATH_TO_SRC} -DCMAKE_BUILD_TYPE=Release
```

You can set the BUILD_TESTING CMake variable to ON if you want to build tests.

Run build

```console
$ cmake --build .
```

When the build is completed, the executables will appear in:

```console
${PATH_TO_BUILD_DIR}/bin
```
