#!/usr/bin/env python3
"""
Image Processing Tool for TaskTown
Handles sprite sheet creation and background removal
"""

import os
import sys
from PIL import Image, ImageChops
import json
import argparse

def remove_complex_background(image_path, output_path=None, method="auto"):
    """Remove complex background (checkerboard, white/gray patterns) from an image"""
    try:
        # Open image
        img = Image.open(image_path).convert("RGBA")
        width, height = img.size
        
        # Get image data as array for easier processing
        import numpy as np
        img_array = np.array(img)
        
        if method == "auto":
            # Auto-detect background type
            method = detect_background_type(img_array)
            print(f"Auto-detected background type: {method}")
        
        if method == "checkerboard":
            # Remove checkerboard pattern (common in image editors)
            img_array = remove_checkerboard_background(img_array)
        elif method == "white_gray":
            # Remove white and light gray backgrounds
            img_array = remove_white_gray_background(img_array)
        elif method == "edge_flood":
            # Use edge-based flood fill to remove background
            img_array = remove_background_flood_fill(img_array)
        else:
            # Default: remove white/light colors
            img_array = remove_white_gray_background(img_array)
        
        # Convert back to PIL Image
        result_img = Image.fromarray(img_array, 'RGBA')
        
        # Save processed image
        if output_path is None:
            name, ext = os.path.splitext(image_path)
            output_path = f"{name}_transparent{ext}"
        
        result_img.save(output_path, "PNG")
        print(f"Processed: {image_path} -> {output_path}")
        return output_path
        
    except Exception as e:
        print(f"Error processing {image_path}: {e}")
        return None

def detect_background_type(img_array):
    """Detect the type of background in the image"""
    height, width = img_array.shape[:2]
    
    # Sample corners to detect background
    corner_samples = [
        img_array[0:10, 0:10],      # Top-left
        img_array[0:10, -10:],      # Top-right  
        img_array[-10:, 0:10],      # Bottom-left
        img_array[-10:, -10:]       # Bottom-right
    ]
    
    # Check for checkerboard pattern
    checkerboard_score = 0
    for corner in corner_samples:
        if has_checkerboard_pattern(corner):
            checkerboard_score += 1
    
    if checkerboard_score >= 2:
        return "checkerboard"
    
    # Check for mostly white/gray background
    light_pixel_count = 0
    total_pixels = 0
    
    for corner in corner_samples:
        for row in corner:
            for pixel in row:
                r, g, b = pixel[:3]
                if r > 200 and g > 200 and b > 200:  # Light colors
                    light_pixel_count += 1
                total_pixels += 1
    
    if light_pixel_count / total_pixels > 0.7:
        return "white_gray"
    
    return "edge_flood"

def has_checkerboard_pattern(corner):
    """Check if a corner has a checkerboard pattern"""
    if corner.shape[0] < 4 or corner.shape[1] < 4:
        return False
    
    # Check for alternating light/dark pattern
    alternating_count = 0
    total_checks = 0
    
    for i in range(0, corner.shape[0]-1, 2):
        for j in range(0, corner.shape[1]-1, 2):
            if i+1 < corner.shape[0] and j+1 < corner.shape[1]:
                # Get 2x2 block
                block = corner[i:i+2, j:j+2]
                if is_checkerboard_block(block):
                    alternating_count += 1
                total_checks += 1
    
    return total_checks > 0 and alternating_count / total_checks > 0.5

def is_checkerboard_block(block):
    """Check if a 2x2 block has checkerboard pattern"""
    if block.shape[0] != 2 or block.shape[1] != 2:
        return False
    
    # Calculate brightness for each pixel
    brightness = []
    for i in range(2):
        for j in range(2):
            r, g, b = block[i, j][:3]
            brightness.append((r + g + b) / 3)
    
    # Check if diagonal pixels are similar and different from others
    diag1_diff = abs(brightness[0] - brightness[3])  # Top-left vs bottom-right
    diag2_diff = abs(brightness[1] - brightness[2])  # Top-right vs bottom-left
    cross_diff = abs(brightness[0] - brightness[1])  # Adjacent pixels
    
    return diag1_diff < 30 and diag2_diff < 30 and cross_diff > 50

def remove_checkerboard_background(img_array):
    """Remove checkerboard background pattern"""
    import numpy as np
    
    height, width = img_array.shape[:2]
    result = img_array.copy()
    
    # Create a mask for background pixels
    background_mask = np.zeros((height, width), dtype=bool)
    
    # Detect checkerboard pattern and mark as background
    for i in range(0, height-1, 2):
        for j in range(0, width-1, 2):
            if i+1 < height and j+1 < width:
                block = img_array[i:i+2, j:j+2]
                if is_checkerboard_block(block):
                    background_mask[i:i+2, j:j+2] = True
    
    # Also mark light pixels as background
    for i in range(height):
        for j in range(width):
            r, g, b = img_array[i, j][:3]
            if r > 220 and g > 220 and b > 220:  # Very light pixels
                background_mask[i, j] = True
    
    # Make background pixels transparent
    result[background_mask] = [0, 0, 0, 0]
    
    return result

def remove_white_gray_background(img_array, tolerance=40):
    """Remove white and gray background with tolerance"""
    import numpy as np
    
    result = img_array.copy()
    height, width = result.shape[:2]
    
    for i in range(height):
        for j in range(width):
            r, g, b = result[i, j][:3]
            
            # Check if pixel is white/gray (similar R, G, B values and bright)
            color_similarity = max(abs(r-g), abs(g-b), abs(r-b))
            brightness = (r + g + b) / 3
            
            if color_similarity < tolerance and brightness > 180:
                result[i, j] = [0, 0, 0, 0]  # Make transparent
    
    return result

def remove_background_flood_fill(img_array):
    """Use flood fill from edges to remove background"""
    import numpy as np
    from collections import deque
    
    height, width = img_array.shape[:2]
    result = img_array.copy()
    visited = np.zeros((height, width), dtype=bool)
    
    # Start flood fill from all edge pixels
    queue = deque()
    
    # Add edge pixels to queue
    for i in range(height):
        for j in range(width):
            if i == 0 or i == height-1 or j == 0 or j == width-1:
                queue.append((i, j))
    
    # Flood fill similar colors from edges
    while queue:
        y, x = queue.popleft()
        
        if visited[y, x]:
            continue
            
        visited[y, x] = True
        seed_color = result[y, x][:3]
        
        # Check if this pixel should be background (light color)
        r, g, b = seed_color
        if not (r > 180 and g > 180 and b > 180):
            continue
            
        # Make this pixel transparent
        result[y, x] = [0, 0, 0, 0]
        
        # Add similar neighboring pixels to queue
        for dy in [-1, 0, 1]:
            for dx in [-1, 0, 1]:
                ny, nx = y + dy, x + dx
                if (0 <= ny < height and 0 <= nx < width and 
                    not visited[ny, nx]):
                    
                    neighbor_color = result[ny, nx][:3]
                    color_diff = sum(abs(a - b) for a, b in zip(seed_color, neighbor_color))
                    
                    if color_diff < 60:  # Similar color
                        queue.append((ny, nx))
    
    return result

def remove_white_background(image_path, output_path=None, tolerance=30):
    """Legacy function - now uses the new complex background removal"""
    return remove_complex_background(image_path, output_path, "white_gray")

def create_sprite_sheet(image_paths, output_path, columns=4, frame_width=32, frame_height=32):
    """Create a sprite sheet from multiple images"""
    try:
        rows = (len(image_paths) + columns - 1) // columns
        sheet_width = columns * frame_width
        sheet_height = rows * frame_height
        
        # Create new sprite sheet
        sprite_sheet = Image.new("RGBA", (sheet_width, sheet_height), (0, 0, 0, 0))
        
        for i, image_path in enumerate(image_paths):
            if not os.path.exists(image_path):
                print(f"Warning: Image not found: {image_path}")
                continue
                
            # Load and resize frame
            frame = Image.open(image_path).convert("RGBA")
            frame = frame.resize((frame_width, frame_height), Image.Resampling.LANCZOS)
            
            # Calculate position
            col = i % columns
            row = i // columns
            x = col * frame_width
            y = row * frame_height
            
            # Paste frame onto sprite sheet
            sprite_sheet.paste(frame, (x, y), frame)
            
        # Save sprite sheet
        sprite_sheet.save(output_path, "PNG")
        print(f"Created sprite sheet: {output_path}")
        print(f"Dimensions: {sheet_width}x{sheet_height}")
        print(f"Frames: {len(image_paths)} ({columns}x{rows})")
        
        return output_path
        
    except Exception as e:
        print(f"Error creating sprite sheet: {e}")
        return None

def process_single_cat_image(image_path, output_path=None):
    """Process a single cat image with complex background removal"""
    if output_path is None:
        name, ext = os.path.splitext(image_path)
        output_path = f"{name}_clean{ext}"
    
    print(f"Processing cat image: {image_path}")
    
    # Try different methods and pick the best result
    methods = ["checkerboard", "white_gray", "edge_flood"]
    best_result = None
    best_method = None
    
    for method in methods:
        try:
            temp_path = f"temp_{method}.png"
            result_path = remove_complex_background(image_path, temp_path, method)
            
            if result_path and os.path.exists(result_path):
                # Load and evaluate result
                result_img = Image.open(result_path)
                transparency_ratio = calculate_transparency_ratio(result_img)
                
                print(f"Method {method}: {transparency_ratio:.2%} transparent")
                
                # Pick method with reasonable transparency (not too much, not too little)
                if 0.3 < transparency_ratio < 0.8:
                    if best_result is None or abs(transparency_ratio - 0.5) < abs(best_result - 0.5):
                        best_result = transparency_ratio
                        best_method = method
                        if os.path.exists(output_path):
                            os.remove(output_path)
                        os.rename(result_path, output_path)
                
                # Clean up temp file
                if os.path.exists(result_path) and result_path != output_path:
                    os.remove(result_path)
                    
        except Exception as e:
            print(f"Method {method} failed: {e}")
    
    if best_method:
        print(f"Best method: {best_method} (transparency: {best_result:.2%})")
        return output_path
    else:
        print("All methods failed, using fallback")
        return remove_complex_background(image_path, output_path, "white_gray")

def calculate_transparency_ratio(img):
    """Calculate the ratio of transparent pixels in an image"""
    if img.mode != 'RGBA':
        return 0.0
    
    import numpy as np
    img_array = np.array(img)
    alpha_channel = img_array[:, :, 3]
    transparent_pixels = np.sum(alpha_channel == 0)
    total_pixels = alpha_channel.size
    
    return transparent_pixels / total_pixels

def process_character_frames(input_dir, output_path="assets/sprites/characters/cat_sprite_sheet.png"):
    """Process character animation frames into a sprite sheet"""
    
    # Get all PNG files from the directory
    png_files = [f for f in os.listdir(input_dir) if f.lower().endswith('.png')]
    png_files.sort()  # Sort alphabetically
    
    if not png_files:
        print(f"No PNG files found in {input_dir}")
        return None
    
    print(f"Found {len(png_files)} PNG files: {png_files}")
    
    # Process each frame to remove complex background
    processed_frames = []
    temp_dir = "temp_processed"
    os.makedirs(temp_dir, exist_ok=True)
    
    for i, frame_file in enumerate(png_files):
        input_path = os.path.join(input_dir, frame_file)
        temp_path = os.path.join(temp_dir, f"frame_{i:02d}.png")
        
        print(f"Processing frame {i+1}/{len(png_files)}: {frame_file}")
        
        # Use the smart cat image processor
        if process_single_cat_image(input_path, temp_path):
            processed_frames.append(temp_path)
        else:
            print(f"Failed to process {frame_file}, creating placeholder")
            # Create a placeholder transparent frame
            placeholder = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
            placeholder.save(temp_path)
            processed_frames.append(temp_path)
    
    if not processed_frames:
        print("No frames were processed successfully")
        return None
    
    # Determine sprite sheet layout
    frame_count = len(processed_frames)
    if frame_count <= 4:
        columns = frame_count
        rows = 1
    elif frame_count <= 8:
        columns = 4
        rows = 2
    elif frame_count <= 16:
        columns = 4
        rows = 4
    else:
        columns = 8
        rows = (frame_count + 7) // 8
    
    print(f"Creating sprite sheet: {columns}x{rows} for {frame_count} frames")
    
    # Create sprite sheet
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    result = create_sprite_sheet(processed_frames, output_path, columns=columns, frame_width=64, frame_height=64)
    
    # Clean up temp files
    for temp_file in processed_frames:
        try:
            os.remove(temp_file)
        except:
            pass
    try:
        os.rmdir(temp_dir)
    except:
        pass
    
    return result

def generate_building_placeholders():
    """Generate placeholder building sprites if none exist"""
    buildings = [
        ("coffee_shop", (255, 165, 0), (96, 128)),
        ("home", (100, 149, 237), (128, 96)),
        ("bulletin_board", (255, 255, 0), (80, 96)),
        ("library", (128, 0, 128), (120, 100)),
        ("gym", (255, 69, 0), (110, 90))
    ]
    
    os.makedirs("assets/sprites/buildings", exist_ok=True)
    
    for building_id, color, size in buildings:
        output_path = f"assets/sprites/buildings/{building_id}.png"
        if not os.path.exists(output_path):
            # Create simple colored rectangle with border
            img = Image.new("RGBA", size, (*color, 255))
            
            # Add simple border
            from PIL import ImageDraw
            draw = ImageDraw.Draw(img)
            draw.rectangle([0, 0, size[0]-1, size[1]-1], outline=(0, 0, 0, 255), width=2)
            
            img.save(output_path)
            print(f"Created placeholder: {output_path}")

def install_dependencies():
    """Install required dependencies"""
    try:
        import numpy
        print("✓ numpy is already installed")
    except ImportError:
        print("Installing numpy...")
        import subprocess
        import sys
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", "--user", "numpy"])
            print("✓ numpy installed successfully")
        except:
            print("❌ Failed to install numpy. Please install manually:")
            print("  pip install --user numpy")
            print("  or: brew install numpy")
            return False
    return True

def main():
    parser = argparse.ArgumentParser(description="Process images for TaskTown")
    parser.add_argument("--remove-bg", help="Remove complex background from image")
    parser.add_argument("--create-sheet", help="Create sprite sheet from directory")
    parser.add_argument("--process-character", help="Process character frames from directory")
    parser.add_argument("--process-single", help="Process a single cat image")
    parser.add_argument("--generate-placeholders", action="store_true", help="Generate placeholder buildings")
    parser.add_argument("--install-deps", action="store_true", help="Install required dependencies")
    parser.add_argument("--output", help="Output path")
    parser.add_argument("--method", choices=["auto", "checkerboard", "white_gray", "edge_flood"], 
                       default="auto", help="Background removal method")
    
    args = parser.parse_args()
    
    if args.install_deps:
        install_dependencies()
        return
    
    # Check dependencies for image processing
    if args.remove_bg or args.process_character or args.process_single:
        if not install_dependencies():
            return
    
    if args.remove_bg:
        remove_complex_background(args.remove_bg, args.output, args.method)
    elif args.process_single:
        output_path = args.output or None
        process_single_cat_image(args.process_single, output_path)
    elif args.create_sheet:
        # Get all PNG files from directory
        image_dir = args.create_sheet
        image_files = [os.path.join(image_dir, f) for f in os.listdir(image_dir) if f.endswith('.png')]
        image_files.sort()
        
        output_path = args.output or "sprite_sheet.png"
        create_sprite_sheet(image_files, output_path)
    elif args.process_character:
        output_path = args.output or "assets/sprites/characters/cat_sprite_sheet.png"
        process_character_frames(args.process_character, output_path)
    elif args.generate_placeholders:
        generate_building_placeholders()
    else:
        print("TaskTown Image Processor")
        print("=======================")
        print()
        print("For your complex cat sprite background:")
        print("  python3 tools/image_processor.py --process-single your_cat_image.png")
        print("  python3 tools/image_processor.py --process-character ./your_cat_frames_folder/")
        print()
        print("Other commands:")
        print("  python3 tools/image_processor.py --remove-bg image.png --method checkerboard")
        print("  python3 tools/image_processor.py --generate-placeholders")
        print("  python3 tools/image_processor.py --install-deps")
        print()
        print("Methods: auto, checkerboard, white_gray, edge_flood")

if __name__ == "__main__":
    main()