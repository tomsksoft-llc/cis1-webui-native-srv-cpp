# cis1 webui development guide

## Repository structure

`include` - C++ header files.

`src` - C++ source files.

`cmake` - CMake scripts.

`test_package` - unit tests.

`docs` - additional documentation.

`version.txt` - package version.

## Backwards compatibility

The WebSocket protocol and all HTTP paths (such as file download links) must be backward compatible at least in one major version.

## Contributing

Follow [the GitHub pull request workflow](https://guides.github.com/introduction/flow/): fork, branch, commit, pull request, automated tests, review, merge.

* Do PR only to dev branch.
* Every PR must have an issue associated with it.
* Try writing smaller commits that do one particular thing only.
* Commit message should briefly describe the code changes.
* Try covering your code with unit tests.
* For extra code-health changes, either submit a separate issue or make it a separate PR that can be easily reviewed.
* Add reviewer to the pull request.

The code style for the C++ code can be found on [this](https://github.com/tomsksoft-llc/cis1-webui-native-srv-cpp/wiki/Codestyle "C++ CodeStyle") wiki-page.

## External libraries

Using external libraries is allowed, but you should check the license compatibility first.
