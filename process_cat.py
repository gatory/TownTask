#!/usr/bin/env python3
"""
Quick Cat Image Processor
专门处理有复杂背景（白色灰色方格）的小猫精灵图
"""

import os
import sys
from PIL import Image
import argparse

def smart_cat_background_removal(image_path, output_path=None):
    """
    智能处理小猫图片的复杂背景
    针对白色/灰色方格背景优化
    """
    try:
        print(f"处理图片: {image_path}")
        
        # 打开图片
        img = Image.open(image_path).convert("RGBA")
        width, height = img.size
        
        # 获取像素数据
        pixels = img.load()
        
        # 分析背景颜色
        background_colors = analyze_background_colors(img)
        print(f"检测到背景颜色: {len(background_colors)} 种")
        
        # 处理每个像素
        processed_count = 0
        for y in range(height):
            for x in range(width):
                r, g, b, a = pixels[x, y]
                
                # 检查是否为背景像素
                if is_background_pixel(r, g, b, background_colors):
                    pixels[x, y] = (0, 0, 0, 0)  # 设为透明
                    processed_count += 1
        
        # 后处理：清理边缘
        img = cleanup_edges(img)
        
        # 保存结果
        if output_path is None:
            name, ext = os.path.splitext(image_path)
            output_path = f"{name}_clean.png"
        
        img.save(output_path, "PNG")
        
        transparency_ratio = processed_count / (width * height)
        print(f"处理完成: {output_path}")
        print(f"透明化像素: {processed_count}/{width*height} ({transparency_ratio:.1%})")
        
        return output_path
        
    except Exception as e:
        print(f"处理失败: {e}")
        return None

def analyze_background_colors(img):
    """分析图片的背景颜色"""
    width, height = img.size
    pixels = img.load()
    
    # 采样边缘像素来确定背景颜色
    edge_colors = []
    
    # 采样四个角落和边缘
    sample_size = min(20, width//4, height//4)
    
    # 左上角
    for x in range(sample_size):
        for y in range(sample_size):
            r, g, b, a = pixels[x, y]
            edge_colors.append((r, g, b))
    
    # 右上角
    for x in range(width-sample_size, width):
        for y in range(sample_size):
            r, g, b, a = pixels[x, y]
            edge_colors.append((r, g, b))
    
    # 左下角
    for x in range(sample_size):
        for y in range(height-sample_size, height):
            r, g, b, a = pixels[x, y]
            edge_colors.append((r, g, b))
    
    # 右下角
    for x in range(width-sample_size, width):
        for y in range(height-sample_size, height):
            r, g, b, a = pixels[x, y]
            edge_colors.append((r, g, b))
    
    # 统计颜色频率
    color_count = {}
    for color in edge_colors:
        # 量化颜色以减少噪声
        quantized = (color[0]//16*16, color[1]//16*16, color[2]//16*16)
        color_count[quantized] = color_count.get(quantized, 0) + 1
    
    # 找出最常见的背景颜色
    background_colors = []
    total_samples = len(edge_colors)
    
    for color, count in color_count.items():
        if count / total_samples > 0.1:  # 出现频率超过10%
            r, g, b = color
            # 只考虑亮色作为背景
            if r > 150 and g > 150 and b > 150:
                background_colors.append(color)
    
    # 如果没有找到明显的背景色，使用默认的白色和灰色
    if not background_colors:
        background_colors = [(255, 255, 255), (192, 192, 192), (224, 224, 224)]
    
    return background_colors

def is_background_pixel(r, g, b, background_colors, tolerance=25):
    """判断像素是否为背景"""
    
    # 检查是否接近已知背景颜色
    for bg_r, bg_g, bg_b in background_colors:
        if (abs(r - bg_r) < tolerance and 
            abs(g - bg_g) < tolerance and 
            abs(b - bg_b) < tolerance):
            return True
    
    # 检查是否为白色/浅灰色
    if r > 200 and g > 200 and b > 200:
        # 检查是否为灰度色（R、G、B相近）
        color_variance = max(abs(r-g), abs(g-b), abs(r-b))
        if color_variance < 30:  # 灰度色
            return True
    
    # 检查棋盘格模式的特征
    brightness = (r + g + b) / 3
    if brightness > 180:  # 亮色
        return True
    
    return False

def cleanup_edges(img):
    """清理边缘的孤立像素"""
    width, height = img.size
    pixels = img.load()
    
    # 创建副本进行处理
    result = img.copy()
    result_pixels = result.load()
    
    # 边缘清理：移除孤立的透明像素周围的杂点
    for y in range(1, height-1):
        for x in range(1, width-1):
            r, g, b, a = pixels[x, y]
            
            if a > 0:  # 非透明像素
                # 检查周围8个像素
                transparent_neighbors = 0
                for dy in [-1, 0, 1]:
                    for dx in [-1, 0, 1]:
                        if dx == 0 and dy == 0:
                            continue
                        nr, ng, nb, na = pixels[x+dx, y+dy]
                        if na == 0:  # 透明
                            transparent_neighbors += 1
                
                # 如果被透明像素包围，且颜色很浅，可能是背景残留
                if transparent_neighbors >= 6:
                    if r > 180 and g > 180 and b > 180:
                        result_pixels[x, y] = (0, 0, 0, 0)
    
    return result

def process_folder(folder_path, output_folder=None):
    """处理文件夹中的所有PNG图片"""
    if not os.path.exists(folder_path):
        print(f"文件夹不存在: {folder_path}")
        return
    
    if output_folder is None:
        output_folder = folder_path + "_processed"
    
    os.makedirs(output_folder, exist_ok=True)
    
    png_files = [f for f in os.listdir(folder_path) if f.lower().endswith('.png')]
    
    if not png_files:
        print(f"在 {folder_path} 中没有找到PNG文件")
        return
    
    print(f"找到 {len(png_files)} 个PNG文件")
    
    processed_files = []
    for i, filename in enumerate(png_files):
        input_path = os.path.join(folder_path, filename)
        output_path = os.path.join(output_folder, filename)
        
        print(f"\\n处理 {i+1}/{len(png_files)}: {filename}")
        
        if smart_cat_background_removal(input_path, output_path):
            processed_files.append(output_path)
    
    print(f"\\n处理完成！")
    print(f"输出文件夹: {output_folder}")
    print(f"成功处理: {len(processed_files)}/{len(png_files)} 个文件")
    
    return processed_files

def main():
    parser = argparse.ArgumentParser(description="处理小猫精灵图的复杂背景")
    parser.add_argument("input", help="输入文件或文件夹路径")
    parser.add_argument("--output", help="输出路径")
    parser.add_argument("--folder", action="store_true", help="处理整个文件夹")
    
    args = parser.parse_args()
    
    if args.folder or os.path.isdir(args.input):
        # 处理文件夹
        process_folder(args.input, args.output)
    else:
        # 处理单个文件
        smart_cat_background_removal(args.input, args.output)

if __name__ == "__main__":
    main()