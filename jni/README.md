# Ikuyo Kotlin Interface (JNI)

<img src="https://media1.tenor.com/m/BxOP1n9xgdIAAAAd/bocchi-bocchi-the-rock.gif" height="200">

> [!WARNING]
> Ikuyo's Kotlin / JNI interface is a work in progress and has not been thoroughly tested.<br>
> **Please do not use it for unknown / user-uploaded inputs.**

Ikuyo provides a JNI interface (via the `ikuyo-jni-arch.dll` provided in all releases).<br>
Additionally, a very simple Kotlin interface is available in this directory.

## Usage

You must build the Ikuyo library with the `ikuyo-jni-arch.dll` (for all platforms you intend to support) in the `src/main/resources` directory or it will error.<br>
When building the CMake project, it is automatically copied into this directory (primary intended for testing).

The Ikuyo JNI binary does not check the Kotlin interface version against it's own; mismatched versions may not work correctly.

An example of the Ikuyo Kotlin interface is available in `src/test/kotlin/IkuyoTest.kt` and all available parameters can be viewed by peeking at `IkuyoProcess` class.