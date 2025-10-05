# TaskTown 美术资源指南

这个指南将帮助你处理游戏中的美术资源，包括地图、建筑和角色精灵图。

## 目录结构

```
assets/
├── config/
│   ├── map_config.json          # 地图和建筑配置
│   └── character_config.json    # 角色动画配置
├── sprites/
│   ├── characters/              # 角色精灵图
│   ├── buildings/               # 建筑精灵图
│   ├── ui/                      # UI元素
│   └── effects/                 # 特效
├── sounds/                      # 音效文件
└── fonts/                       # 字体文件
```

## 1. 处理角色精灵图（解决白色背景问题）

### 问题：你的小猫精灵图有白色背景

**解决方案：使用图片处理工具自动去除白色背景**

```bash
# 安装Python依赖
pip install Pillow

# 处理单个图片去除白色背景
python tools/image_processor.py --remove-bg your_cat_frame.png --output cat_frame_transparent.png

# 处理整个文件夹的角色帧并创建精灵图表
python tools/image_processor.py --process-character ./your_cat_frames_folder/
```

### 手动处理步骤：

1. **准备你的角色帧文件**，命名如下：
   ```
   idle_0.png
   walk_down_1.png, walk_down_2.png, walk_down_3.png
   walk_up_0.png, walk_up_1.png, walk_up_2.png, walk_up_3.png
   walk_left_0.png, walk_left_1.png, walk_left_2.png, walk_left_3.png
   walk_right_0.png, walk_right_1.png, walk_right_2.png, walk_right_3.png
   ```

2. **运行处理脚本**：
   ```bash
   python tools/image_processor.py --process-character ./your_frames_folder/
   ```

3. **结果**：会在 `assets/sprites/characters/cat_sprite_sheet.png` 生成一个4x4的精灵图表

## 2. 处理建筑和地图

### 问题：AI生成的地图如何确定建筑位置

**解决方案：使用配置文件系统**

1. **编辑地图配置** (`assets/config/map_config.json`)：
   ```json
   {
     "buildings": [
       {
         "id": "coffee_shop",
         "name": "Coffee Shop", 
         "position": { "x": 200, "y": 200 },
         "size": { "width": 96, "height": 128 },
         "spriteFile": "buildings/coffee_shop.png"
       }
     ]
   }
   ```

2. **放置建筑精灵图**：
   - 将建筑图片放在 `assets/sprites/buildings/` 文件夹
   - 文件名要与配置中的 `spriteFile` 匹配

3. **如果没有建筑图片**：
   ```bash
   # 生成占位符建筑
   python tools/image_processor.py --generate-placeholders
   ```

## 3. 实际使用流程

### 场景1：你有复杂的AI生成地图

1. **分析地图**：确定建筑在图片中的位置
2. **裁剪建筑**：从大图中裁剪出单个建筑
3. **更新配置**：在 `map_config.json` 中设置正确的位置和大小
4. **测试**：运行游戏查看效果

### 场景2：你有白色背景的角色帧

1. **整理帧文件**：按动画类型命名
2. **运行处理脚本**：
   ```bash
   python tools/image_processor.py --process-character ./your_frames/
   ```
3. **检查结果**：查看生成的精灵图表
4. **调整配置**：如需要，修改 `character_config.json`

## 4. 配置文件详解

### 地图配置 (map_config.json)

```json
{
  "map": {
    "width": 1024,        // 地图宽度
    "height": 768,        // 地图高度
    "tileSize": 64        // 瓦片大小
  },
  "buildings": [
    {
      "id": "unique_id",           // 唯一标识符
      "name": "Display Name",      // 显示名称
      "position": { "x": 200, "y": 200 },  // 屏幕位置
      "size": { "width": 96, "height": 128 }, // 建筑大小
      "spriteFile": "buildings/building.png", // 精灵图文件
      "color": { "r": 255, "g": 165, "b": 0, "a": 255 }, // 备用颜色
      "interactionRadius": 50,     // 交互半径
      "type": "pomodoro"          // 建筑类型
    }
  ]
}
```

### 角色配置 (character_config.json)

```json
{
  "character": {
    "spriteSheet": "characters/cat_sprite_sheet.png",
    "frameSize": { "width": 32, "height": 32 },
    "animations": {
      "idle": {
        "frames": [0],           // 帧索引
        "frameRate": 1,          // 帧率
        "loop": true             // 是否循环
      },
      "walk_down": {
        "frames": [0, 1, 2, 3],
        "frameRate": 8,
        "loop": true
      }
    }
  }
}
```

## 5. 常见问题解决

### Q: 角色显示为白色方块
**A**: 白色背景没有被正确移除
- 使用 `--remove-bg` 工具处理图片
- 检查容差设置（tolerance参数）

### Q: 建筑位置不对
**A**: 配置文件中的位置需要调整
- 编辑 `map_config.json` 中的 `position` 值
- 重新运行游戏测试

### Q: 动画不流畅
**A**: 帧率或帧顺序问题
- 调整 `character_config.json` 中的 `frameRate`
- 检查 `frames` 数组的顺序

### Q: 图片加载失败
**A**: 文件路径或格式问题
- 确保文件路径正确
- 使用PNG格式（支持透明度）
- 检查文件权限

## 6. 高级技巧

### 批量处理图片
```bash
# 处理文件夹中所有图片去除白色背景
for file in *.png; do
    python tools/image_processor.py --remove-bg "$file" --output "processed_$file"
done
```

### 自定义背景颜色移除
修改 `image_processor.py` 中的 `remove_white_background` 函数，调整颜色和容差值。

### 动态调整建筑位置
在游戏运行时按住Ctrl+D可以显示调试信息，帮助你确定正确的坐标。

## 7. 工作流程建议

1. **先用占位符**：使用简单的彩色矩形开始开发
2. **逐步替换**：有了美术资源后逐个替换
3. **配置驱动**：通过修改JSON文件调整，无需重新编译
4. **版本控制**：将配置文件加入git，但大的图片文件可以用Git LFS

这样你就可以轻松管理所有的美术资源了！