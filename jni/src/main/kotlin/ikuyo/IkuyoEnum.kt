package ikuyo

enum class IkuyoTextureContainer(val value: Int) {
    Unknown(0),
    DDS(1), UnityAssetBundle(2),
    UE4(3), FArC(4), TXP(5)
}
enum class IkuyoImageContainer(val value: Int) {
    PNG(1), WebP(2), AVIF(3)
}