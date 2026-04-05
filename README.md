# ikuyo ・ いくよ

ikuyo is a texture decoder for several rhythm arcade (or arcade-adjacent) games and their engines

![kitaura](https://media1.tenor.com/m/6haixoTOSwcAAAAC/bocchi-bocchi-the-rock.gif)

## Supported Formats

### Input

- DDS (DirectDraw Surface) with DXT1/3/5 or RGB/RGBA
- UE4 Texture (.uexp, specifically 4.19.2)
- TXP (only via .farc)

### Output

- PNG (via fpng), fast but worse compression
- WebP (via libwebp), slow but better compression
- AVIF (via libavif/SV1-AV1), slowest but best compression

## Usage

> [!IMPORTANT]
> Windows users will need to install the [Visual Studio C++ Redistributable](https://aka.ms/vc14/vc_redist.x64.exe) to run the pre-built binaries.

Binaries can be found in the [releases tab](https://github.com/raymonable/ikuyo/releases/latest) for Windows and Linux.<br>
macOS users may need to use a solution such as [Wine](https://winehq.org) (preferred) or [Karton](https://karton.github.io) (untested).

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

## Acknowledgements

- All external libraries (excluding AVIF encoders/decoders) are available to view via the [external](./external) directory
- The Unity implementation is derived from [Razviar's AssetStudio fork](https://github.com/Razviar/assetstudio)
- Project DIVA TXP implementation referenced [diva-rust-modding txp library](https://github.com/diva-rust-modding/txp)

Thank you to everyone who has contributed to ikuyo and/or it's dependencies.

<hr>
<sub>The *BOCCHI THE ROCK!* animation and derivatives are property of CloverWorks and Aki Hamazi. All rights belong to their respective owners. No copyright infringement is intended.</sub>
