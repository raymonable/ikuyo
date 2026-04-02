# ikuyo ・ いくよ

ikuyo is a texture decoder for several SEGA-like games

## Supported Formats

### Input

- DDS (DirectDraw Surface) with DXT1/3/5 or RGB/RGBA
- UE4 Texture (.uexp, specifically 4.19.2)

### Output

- PNG (via fpng), fast but worse compression
- WebP (via libwebp), slow but better compression