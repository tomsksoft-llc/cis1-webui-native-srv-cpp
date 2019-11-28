# cis1 webui development guide

## Repository structure

`include` - C++ header files.

`src` - C++ source files.

`cmake` - CMake scripts.

`test_package` - unit tests.

`docs` - additional documentation.

`version.txt` - package version.

## Backwards compatibility

WebSocket protocol and all HTTP paths (like file download links) must be backward compatible at least in one major version.

## Contributing

Follow [the GitHub pull request workflow](https://guides.github.com/introduction/flow/): fork, branch, commit, pull request, automated tests, review, merge.

* Do PR only to dev branch.
* Every PR must have issue associated with it.
* Try to write smaller commits, that do exactly one thing.
* Commit message should briefly describe changes in code.
* Try to cover your code with unit tests.
* For extra code-health changes, either file a separate issue, or make it a separate PR that can be easily reviewed.
* Add reviewer to pull request.

Code style for C++ code can be found on [this](https://github.com/tomsksoft-llc/cis1-webui-native-srv-cpp/wiki/Codestyle "C++ CodeStyle") wiki-page.

## External libraries

External libraries usage is allowed, but you should check license compatibility.
