package ikuyo

import java.nio.ByteBuffer
import java.nio.ByteOrder

class IkuyoProcess {
    var formatInput: IkuyoTextureContainer = IkuyoTextureContainer.Unknown;
    var formatOutput: IkuyoImageContainer = IkuyoImageContainer.PNG;

    var width: Int? = null;
    var height: Int? = null;

    var preferredImageIndex: Int? = null;
    var preferredQuality: Int = 75;
}

class Ikuyo {
    init {
        val libraryName = System.mapLibraryName("ikuyo-jni");
        val library = kotlin.io.path.createTempFile(
            libraryName.split('.').last()
        ).toFile();

        (javaClass.getResourceAsStream("/${libraryName}")
            ?: error("Ikuyo JNI not available for this platform, please rebuild with native library in Ikuyo resources")
        ).use { input ->
            library.outputStream().use { output -> input.copyTo(output) } };

        System.load(library.absolutePath);
    }

    fun process(input: ByteArray, processSettings: IkuyoProcess): Array<ByteBuffer>? {
        val inputBuffer = ByteBuffer.allocateDirect(input.size).put(input)
        return process(inputBuffer, processSettings)
    }

    external fun process(input: ByteBuffer, processSettings: IkuyoProcess): Array<ByteBuffer>?
}