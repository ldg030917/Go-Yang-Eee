import sys
import os
from PIL import Image

def generate_uv_and_template(original_sheet_path, output_uv_path, output_template_path):
    """
    [1. 초기 세팅용]
    기존 스프라이트 시트를 분석하여 UV 맵과 전개도(Template)를 추출합니다.
    - 같은 색상끼리 묶어서 전개도에 배치합니다.
    """
    try:
        img = Image.open(original_sheet_path).convert("RGBA")
    except Exception as e:
        print(f"Error opening image: {e}")
        return

    width, height = img.size
    pixels = img.load()

    # UV 맵 (R=TemplateX, G=TemplateY, B=0, A=Mask)
    uv_map = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    uv_pixels = uv_map.load()

    # 전개도 (256x256 권장)
    template_size = 256
    template = Image.new("RGBA", (template_size, template_size), (0, 0, 0, 0))
    template_pixels = template.load()

    # 색상별로 전개도에 배치하기 위한 좌표 추적
    # Key: (R,G,B,A) Color, Value: (TemplateX, TemplateY)
    color_to_uv = {}
    
    current_x = 0
    current_y = 0
    next_row_h = 0

    print("Generating UV Map and Template...")

    for y in range(height):
        for x in range(width):
            color = pixels[x, y]
            
            # 투명한 픽셀은 스킵
            if color[3] == 0:
                continue

            # 이미 전개도에 등록된 색상인가?
            if color in color_to_uv:
                u, v = color_to_uv[color]
            else:
                # 새로운 색상이면 전개도에 추가 (단순 그리드 배치)
                # 실제로는 아티스트가 나중에 전개도를 예쁘게 재배치해야 함
                if current_x >= template_size:
                    current_x = 0
                    current_y += 1 # 1픽셀씩 줄바꿈 (나중에 정리 필요)
                
                if current_y >= template_size:
                    print("Warning: Template size too small for all unique colors!")
                    break

                u, v = current_x, current_y
                
                # 전개도에 점 찍기
                template_pixels[u, v] = color
                color_to_uv[color] = (u, v)

                # 다음 위치 (일단 1자로 쭉 나열)
                current_x += 1

            # UV 맵에 기록 (R=u, G=v)
            uv_pixels[x, y] = (u, v, 0, 255)

    uv_map.save(output_uv_path)
    template.save(output_template_path)
    print(f"Done! Saved {output_uv_path} and {output_template_path}")
    print(f"Total unique colors found: {len(color_to_uv)}")


def bake_sprite_sheet(uv_map_path, new_skin_path, output_sheet_path):
    """
    [2. 작업용/빌드용]
    UV 맵과 새로운 스킨(전개도)을 합성하여 최종 스프라이트 시트를 생성합니다.
    """
    try:
        uv_img = Image.open(uv_map_path).convert("RGBA")
        skin_img = Image.open(new_skin_path).convert("RGBA")
    except Exception as e:
        print(f"Error: {e}")
        return

    width, height = uv_img.size
    uv_pixels = uv_img.load()
    skin_pixels = skin_img.load()
    
    skin_w, skin_h = skin_img.size

    # 결과물 이미지
    output = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    out_pixels = output.load()

    print(f"Baking {output_sheet_path}...")

    for y in range(height):
        for x in range(width):
            uv_color = uv_pixels[x, y]
            
            # UV맵의 Alpha가 0이면 투명 처리
            if uv_color[3] == 0:
                continue

            # R -> U (Skin X), G -> V (Skin Y)
            u = uv_color[0]
            v = uv_color[1]

            # 범위 체크
            if 0 <= u < skin_w and 0 <= v < skin_h:
                # 스킨 색상을 가져와서 시트에 칠함
                out_pixels[x, y] = skin_pixels[u, v]

    output.save(output_sheet_path)
    print("Baking Complete!")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage:")
        print("1. Extract UV & Template: python SkinBaker.py extract <original.png> <uv.png> <template.png>")
        print("2. Bake Sprite Sheet:     python SkinBaker.py bake <uv.png> <my_skin.png> <output.png>")
        sys.exit(1)

    mode = sys.argv[1]

    if mode == "extract" and len(sys.argv) == 5:
        generate_uv_and_template(sys.argv[2], sys.argv[3], sys.argv[4])
    elif mode == "bake" and len(sys.argv) == 5:
        bake_sprite_sheet(sys.argv[2], sys.argv[3], sys.argv[4])
    else:
        print("Invalid arguments.")
