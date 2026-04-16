import sys
from PIL import Image

def convert_to_rgb565(png_path, name):
    img = Image.open(png_path).convert('RGB')
    width, height = img.size
    
    print(f"/* Image: {name} ({width}x{height}) */")
    print(f"const uint16_t {name}[] = {{")
    
    pixels = []
    for y in range(height):
        row = []
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            # Convert 8-bit R,G,B to 5-bit R, 6-bit G, 5-bit B
            rgb565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            row.append(f"0x{rgb565:04X}")
        pixels.append(", ".join(row))
    
    print(",\n".join(pixels))
    print("};")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python converter.py image.png array_name")
    else:
        convert_to_rgb565(sys.argv[1], sys.argv[2])