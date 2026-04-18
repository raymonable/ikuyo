import ikuyo.*
import java.io.File
import kotlin.jvm.javaClass

fun run(ikuyo: Ikuyo, filename: String) {
    val texture = object {}.javaClass.getResource("/${filename}")?.readBytes()
        ?: error("failed to read texture")

    val imageBuffers = ikuyo.process(texture, IkuyoProcess().apply {
        formatOutput = IkuyoImageContainer.AVIF
        width = 1000
        preferredImageIndex = 0
    })
    if (imageBuffers != null) {
        val buffer = imageBuffers.getOrNull(0)
        if (buffer != null) {
            val bytes = ByteArray(buffer.remaining())
            buffer.get(bytes)
            File("${filename}.avif").writeBytes(bytes)
        }
    } else error("failed to process texture");
}

fun main(args: Array<String>) {
    val ikuyo = Ikuyo();

    // BEGIN: test
    run(ikuyo, "test.dds")
    run(ikuyo, "test.bin")
}