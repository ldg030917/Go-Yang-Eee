from PIL import Image

def create_uv_reference(size=256, filename="uv_reference.png"):
    # RGBA 모드, 크기 256x256
    img = Image.new("RGBA", (size, size))
    pixels = img.load()

    for y in range(size):
        for x in range(size):
            # R = x좌표 (0~255)
            # G = y좌표 (0~255)
            # B = 0 (사용 안함)
            # A = 255 (불투명)
            pixels[x, y] = (x, y, 0, 255)

    img.save(filename)
    print(f"Generated {filename} ({size}x{size})")

if __name__ == "__main__":
    create_uv_reference()
