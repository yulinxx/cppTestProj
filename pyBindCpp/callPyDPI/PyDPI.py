# dpi_extractor.py
from PIL import Image

def get_image_dpi(image_path):
    # 打开图片文件
    img = Image.open(image_path)

    # 获取图片的DPI
    dpi = img.info.get('dpi')

    # 如果DPI信息不存在，则设置默认值
    if dpi is None or dpi == (0, 0):
        dpi = (96, 96)  # 设置默认DPI值，例如96 DPI

    # 打印DPI信息
    if dpi:
        print(f"图片的DPI为：{dpi}")
    else:
        print("未找到DPI信息")

    return dpi