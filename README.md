# VybeSC

Generated from <https://github.com/supercollider/cookiecutter-supercollider-plugin>.

Author: Paulo Feodrippe

TBD

### Requirements

- CMake >= 3.5
- SuperCollider source code

### Building

Clone the project:

    git clone https://github.com/pfeodrippe/vybesc
    cd vybesc
    mkdir build
    cd build

Then, use CMake to configure and build it:

    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . --config Release
    cmake --build . --config Release --target install

For debug:

    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/vybe-games/vybe_native/macos/universal/supercollider/Resources/plugins

You may want to manually specify the install location in the first step to point it at your
SuperCollider extensions directory: add the option `-DCMAKE_INSTALL_PREFIX=/path/to/extensions`.

It's expected that the SuperCollider repo is cloned at `../supercollider` relative to this repo. If
it's not: add the option `-DSC_PATH=/path/to/sc/source`.

### Developing

Use the command in `regenerate` to update CMakeLists.txt when you add or remove files from the
project. You don't need to run it if you only change the contents of existing files. You may need to
edit the command if you add, remove, or rename plugins, to match the new plugin paths. Run the
script with `--help` to see all available options.
