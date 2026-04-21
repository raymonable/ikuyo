# Ikuyo ・ いくよ

Ikuyo is a texture decoder for several rhythm arcade (or arcade-adjacent) games and their engines

<img src="https://media1.tenor.com/m/6haixoTOSwcAAAAC/bocchi-bocchi-the-rock.gif" height="200">

> [!WARNING]
> Ikuyo is a rapidly expanding and changing library; more formats are planned to be supported and ABI is constantly changing.<br>
> Please wait for a Version 1.0.0 release before deploying into any production libraries or services.

## Supported Formats

### Input

- DDS (DirectDraw Surface) with DXT1/3/5 or RGB/RGBA
- UE4 Texture (.uexp, specifically 4.19.2)

### Output

- PNG (via fpng), fast but worse compression
- WebP (via libwebp), slow but better compression

## Usage

> [!IMPORTANT]
> Windows users will need to install the [Visual Studio C++ Redistributable](https://aka.ms/vc14/vc_redist.x64.exe) to run the pre-built binaries.

Binaries can be found in the [releases tab](https://github.com/raymonable/ikuyo/releases/latest) for all major platforms.<br>

The command-line interface currently has the following arguments:
```
ikuyo [format, ex: dds] [input file] [output format, ex: png] (optional width) (optional height)
```
Compression and such factors currently cannot be modified via the command line.

## Building

### Windows

This assumes you have Visual Studio 2022 with C++ development components installed.

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### Unix

CMake and gcc are required to build ikuyo. Package manager one-liners are provided for common distributions:
- Arch: `sudo pacman -Sy base-devel`
- Debian: `sudo apt install build-essential cmake`
- Fedora: `sudo dnf install gcc-c++ cmake make`

```bash
cmake -S . -B build -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build build 
```

### macOS

You will need XCode and CMake. It will compile for both x86_64 and arm64 by default.
```
cmake -S . -B build -DCMAKE_SYSTEM_NAME=Dawrin -DCMAKE_BUILD_TYPE=Release
cmake --build build 
```

## Future Goals & Tasks

### Tasks

 - Finish the UnityFS implementations
 - Improve API to only sometimes decode textures (intended for parsing large UnityFS bundles, maybe)
 - Improve Kotlin interface and provide better examples
 - Read the sprite information in TXP to separate the jacket from the background in some circumstances
 - Reduce memory leaks (and eventually make Ikuyo entirely memory-safe!)

### Goals

 - GPU decoding on supported targets and formats (such as CLI & BCx?)
 - Include raw binary formats for exporting images
 - Improve ABI and overall stability between versions (post-1.0.0 goal)

## Acknowledgements

- All external libraries (excluding AVIF encoders/decoders) are available to view via the [external](./external) directory
- The Unity implementation is derived from [Razviar's AssetStudio fork](https://github.com/Razviar/assetstudio)
- Project DIVA TXP implementation referenced [diva-rust-modding txp library](https://github.com/diva-rust-modding/txp)

Thank you to everyone who has contributed to Ikuyo and/or it's dependencies.

<hr>
<sub>The <i>BOCCHI THE ROCK!</i> animation and derivatives are property of CloverWorks and Aki Hamazi. All rights belong to their respective owners. No copyright infringement is intended.</sub>
