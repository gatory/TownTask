# TaskTown 美术资源快速开始指南

## 你的问题解决方案

### 1. 白色背景的小猫精灵图问题 ✅ 已解决

**问题**：你的小猫关键帧有白色背景，看起来像白色方块里有个小猫

**解决方案**：
```bash
# 处理单个图片去除白色背景
python3 tools/image_processor.py --remove-bg your_cat_frame.png

# 处理整个文件夹并创建精灵图表
python3 tools/image_processor.py --process-character ./your_cat_frames_folder/
```

### 2. AI生成地图的建筑位置问题 ✅ 已解决

**问题**：如何确定AI生成图片中建筑的位置

**解决方案**：使用配置文件系统
- 编辑 `assets/config/map_config.json` 设置建筑位置
- 程序会根据配置文件自动放置建筑
- 可以实时调整位置无需重新编译

## 快速测试

### 1. 运行带资源管理的版本

```bash
# 编译新版本（包含资源管理）
# 先备份当前main.cpp
mv src/main.cpp src/main_simple.cpp
mv src/main_with_assets.cpp src/main.cpp

# 编译运行
./build.sh
```

### 2. 你会看到什么

- ✅ 建筑现在有彩色的占位符图片（不再是纯色矩形）
- ✅ 建筑位置由配置文件控制
- ✅ 显示建筑类型信息
- ✅ 准备好接受你的真实美术资源

## 添加你的美术资源

### 步骤1：处理小猫精灵图

1. **准备你的小猫帧文件**，放在一个文件夹里
2. **运行处理脚本**：
   ```bash
   python3 tools/image_processor.py --process-character ./你的小猫帧文件夹/
   ```
3. **结果**：会生成 `assets/sprites/characters/cat_sprite_sheet.png`

### 步骤2：添加建筑图片

1. **将建筑图片放入** `assets/sprites/buildings/` 文件夹
2. **命名要匹配配置文件**：
   - `coffee_shop.png`
   - `home.png` 
   - `bulletin_board.png`
   - `library.png`
   - `gym.png`

### 步骤3：调整建筑位置

编辑 `assets/config/map_config.json`：
```json
{
  "buildings": [
    {
      "id": "coffee_shop",
      "position": { "x": 200, "y": 200 },  // 调整这里的位置
      "size": { "width": 96, "height": 128 }  // 调整大小
    }
  ]
}
```

## 实际工作流程

### 如果你有AI生成的完整地图：

1. **分析地图**：确定每个建筑在图片中的位置
2. **裁剪建筑**：从大图中裁剪出单个建筑保存为PNG
3. **更新配置**：在配置文件中设置对应的位置和大小
4. **测试调整**：运行游戏，微调位置直到满意

### 如果你有分离的建筑图片：

1. **去除背景**：
   ```bash
   python3 tools/image_processor.py --remove-bg building.png
   ```
2. **放入正确位置**：复制到 `assets/sprites/buildings/`
3. **配置位置**：编辑JSON文件设置在游戏中的位置

## 配置文件说明

### 建筑配置 (`assets/config/map_config.json`)

```json
{
  "buildings": [
    {
      "id": "coffee_shop",           // 唯一ID
      "name": "Coffee Shop",         // 显示名称
      "position": { "x": 200, "y": 200 },    // 屏幕位置（像素）
      "size": { "width": 96, "height": 128 }, // 建筑大小
      "spriteFile": "buildings/coffee_shop.png", // 图片文件
      "type": "pomodoro"            // 建筑功能类型
    }
  ]
}
```

### 角色配置 (`assets/config/character_config.json`)

```json
{
  "character": {
    "spriteSheet": "characters/cat_sprite_sheet.png",
    "frameSize": { "width": 32, "height": 32 },
    "animations": {
      "walk_down": {
        "frames": [0, 1, 2, 3],    // 精灵图表中的帧索引
        "frameRate": 8,            // 每秒帧数
        "loop": true               // 是否循环
      }
    }
  }
}
```

## 调试技巧

### 查看建筑位置
- 游戏中显示角色坐标
- 可以用这个来确定建筑应该放在哪里

### 测试图片加载
- 如果图片加载失败，会显示彩色矩形作为备用
- 检查控制台输出的错误信息

### 实时调整
- 修改JSON配置文件后重启游戏即可看到效果
- 不需要重新编译代码

## 下一步

1. **测试当前系统**：运行游戏看看占位符效果
2. **准备你的美术资源**：整理小猫帧和建筑图片
3. **使用处理工具**：去除白色背景，创建精灵图表
4. **逐步替换**：一个一个替换占位符资源
5. **微调配置**：调整位置和大小直到满意

这样你就可以轻松管理所有美术资源，而且可以随时调整而不需要修改代码！