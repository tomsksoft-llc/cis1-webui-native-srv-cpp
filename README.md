# Cis1 webui server

> Continuous integration system webui server written in C++

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

This repository contains a webserver for continuous integration system cis1.
The server written in C++.
This component follows the rules described in the repository
[cis1-docs](https://github.com/tomsksoft-llc/cis1-docs/ "cis1 documentation").
This component is compatible with OS Linux.
Working on OS Windows is not guaranteed, but we are working on it.

## Description

This webserver is a part of the cis1 system.
It gives the opportunity to remote control the core and adds additional functionality:
1. Authentication, authorization, user management
2. Projects access rights
3. Possibility to use WebHooks
4. Automation of some routine actions

All interoperation between frontend and webserver, except for file operations,
are carried out via WebSocket.

Default WebSocket address is:

```
ws://${SERVER_IP_ADDRESS}:${SERVER_PORT}/ws
```

Protocol which used over WebSocket is described on
[this](https://github.com/tomsksoft-llc/cis1-webui-native-srv-cpp/wiki/Protocol-description "Protocol description")
wiki page.

## Usage

Copy executable in any directory.

Change `config_example.ini` as you need.

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

Prefer absolute paths.

Run

```console
$ ./cis1_srv ${PATH_TO_CONFIG}/config_example.ini
```

After all previously steps will be completed the web server will be running on configured ip and port.
There is reference implementation of frontend for this web server, it is located in
[this](https://github.com/tomsksoft-llc/cis1-webui-native-front-vanilla "cis1 webui front vanilla")
repository.
You can just download it in `www` directory, and change the WebSocket address in frontend configuration.

WebHooks usage tutorial is presented on
[this](https://github.com/tomsksoft-llc/cis1-webui-native-srv-cpp/wiki/Webhooks-tutorial "Webhooks tutorial")
wiki page.

## Compiling

### Linux

Build requirements:
> Conan, gcc 8.3, CMake 3.9+

Add repositories `tomsksoft` and `bincrafters` to conan

```console
$ conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
$ conan remote add tomsksoft https://api.bintray.com/conan/tomsksoft/cis1
```

Change into the build directory and install build dependencies

```console
$ conan install ${PATH_TO_SRC} --profile -s build_type=Release --build=missing
```

Generate build scripts with CMake

```console
$ cmake ${PATH_TO_SRC} -DCMAKE_BUILD_TYPE=Release
```

You can set CMake variable BUILD_TESTING to ON if you want to build tests.

Run build

```console
$ cmake --build .
```

After build successfuly finished executables will appear in:

```console
${PATH_TO_BUILD_DIR}/bin
```
