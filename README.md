# PyDNG Python 绑定

本项目提供了 Adobe DNG SDK 的 Python 绑定，允许您在 Python 中读取和写入 DNG (Digital Negative) 文件。

## 功能特性

- ✅ 读取 DNG 文件
- ✅ 写入 DNG 文件
- ✅ 提取元数据（EXIF、相机信息等）
- ✅ 获取图像数据（支持 Stage1 和 Stage3）
- ✅ 设置图像数据（从 NumPy 数组）
- ✅ 设置和获取基准曝光值
- ✅ 与 NumPy 无缝集成

## 快速开始

### 使用 pip 安装（推荐）

最简单的方式是通过 pip 一键安装：

```bash
# 从源码安装
pip install .

# 或开发模式安装（可编辑模式）
pip install -e .

# 从 Git 仓库安装
pip install git+https://github.com/yourusername/PyDNG.git
```

pip 会自动处理所有依赖和编译过程。详细说明请查看 [README_PIP.md](README_PIP.md)。

### 手动构建

如果需要手动控制构建过程：

## 构建要求

- CMake 3.15 或更高版本
- Python 3.7 或更高版本（包含开发头文件）
- C++14 兼容的编译器
- pybind11（作为 git submodule 或自动下载）

## 构建步骤

### Windows

```bash
# 创建构建目录
mkdir build
cd build

# 配置 CMake（启用 Python 绑定）
cmake .. -DBUILD_PYTHON_BINDINGS=ON

# 构建
cmake --build . --config Release

# Python 模块将位于 build/python/ 目录
```

### Linux/macOS

```bash
# 创建构建目录
mkdir build
cd build

# 配置 CMake
cmake .. -DBUILD_PYTHON_BINDINGS=ON -DCMAKE_BUILD_TYPE=Release

# 构建
make -j$(nproc)

# Python 模块将自动复制到 Python site-packages 目录
```

## 使用方法

### 基本用法

```python
import pydng
import numpy as np

# 创建 Dng 对象
dng = pydng.Dng()

# 读取 DNG 文件
error_code = dng.read("input.dng", ignore_enhanced=False)

if error_code == pydng.ErrorCode.NONE:
    # 获取元数据
    meta = dng.get_meta()
    print(f"Camera: {meta.make} {meta.model}")
    print(f"Image Size: {meta.width} x {meta.height}")
    print(f"ISO: {meta.iso}")
    print(f"Exposure Time: {meta.exposure_time} sec")
    
    # 获取图像数据
    data = dng.get_data(enhanced=False)
    numpy_array = data.to_numpy()
    print(f"Image shape: {numpy_array.shape}")
```

### 写入 DNG 文件

```python
import pydng
import numpy as np

# 创建示例图像（16位 RGB）
height, width, channels = 1000, 1500, 3
image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)

# 创建 Dng 对象
dng = pydng.Dng()

# 设置图像数据（3 = ttShort, 16位无符号整数）
dng.set_data(image_data, 3, enhanced=False)

# 设置元数据
meta = pydng.DngMeta()
meta.make = "My Camera"
meta.model = "Example"
meta.width = width
meta.height = height
meta.iso = 100
meta.exposure_time = 1.0 / 60.0
meta.f_number = 2.8
meta.focal_length = 50.0

dng.set_meta(meta)

# 写入文件
error_code = dng.write("output.dng")
```

## API 参考

### Dng 类

主要类，用于读取和写入 DNG 文件。

#### 方法

- `read(path: str, ignore_enhanced: bool = False) -> ErrorCode`
    - 读取 DNG 文件

- `write(path: str) -> ErrorCode`
    - 写入 DNG 文件

- `get_data(enhanced: bool = False) -> DngData`
    - 获取图像数据
    - `enhanced=True` 返回 Stage3 图像，`False` 返回 Stage1 图像

- `set_data(data: np.ndarray, pixel_type: int, enhanced: bool = False) -> None`
    - 设置图像数据
    - `data`: NumPy 数组，形状为 (height, width, channels)
    - `pixel_type`: 像素类型数值（1=ttByte, 3=ttShort, 8=ttSShort, 4=ttLong）

- `get_meta() -> DngMeta`
    - 获取元数据

- `set_meta(meta: DngMeta) -> None`
    - 设置元数据

- `get_baseline_exposure() -> float`
    - 获取基准曝光值

- `set_baseline_exposure(exposure: float) -> None`
    - 设置基准曝光值

### DngMeta 类

包含 DNG 文件的元数据信息。

#### 属性

- `make`: 相机厂商
- `model`: 相机型号
- `software`: 软件信息
- `artist`: 艺术家
- `copyright`: 版权信息
- `width`, `height`: 图像尺寸
- `raw_width`, `raw_height`: RAW 图像尺寸
- `exposure_time`: 曝光时间（秒）
- `f_number`: 光圈值
- `focal_length`: 焦距（mm）
- `iso`: ISO 感光度
- `focal_length_35mm`: 35mm 等效焦距
- `date_time`: 拍摄日期时间
- `date_time_original`: 原始拍摄日期时间
- `is_monochrome`: 是否为单色图像
- `color_planes`: 颜色平面数
- `color_space`: 色彩空间

### DngData 类

包含图像数据。

#### 属性

- `width`, `height`: 图像尺寸
- `channels`: 通道数
- `pixel_type`: 像素类型
- `top`, `left`: 活动区域偏移

#### 方法

- `to_numpy() -> np.ndarray`
    - 转换为 NumPy 数组

### 常量

- `ErrorCode.NONE`: 无错误
- `ErrorCode.READ_FILE`: 读取文件错误
- `ErrorCode.WRITE_FILE`: 写入文件错误
- `ErrorCode.BAD_FORMAT`: 格式错误
- `ErrorCode.UNKNOWN`: 未知错误

像素类型数值（参考 [PIXEL_TYPES.md](PIXEL_TYPES.md)）：
- `1` = ttByte (8位无符号整数)
- `3` = ttShort (16位无符号整数)
- `8` = ttSShort (16位有符号整数)
- `4` = ttLong (32位无符号整数)

## 示例

查看 `examples/` 目录中的示例代码：

- `example_read_dng.py`: 读取 DNG 文件并显示信息
- `example_write_dng.py`: 创建并写入 DNG 文件

## 注意事项

1. **内存管理**: `get_data()` 返回的 `DngData` 对象会在转换为 NumPy 数组时自动管理内存。不要手动释放指针。

2. **像素类型**: 确保 `set_data()` 时使用的 `pixel_type` 与 NumPy 数组的数据类型匹配。

3. **图像格式**: 图像数据应为形状 `(height, width, channels)` 的 NumPy 数组。

4. **Windows 路径**: 在 Windows 上，路径会自动转换为宽字符格式。

## 故障排除

### 导入错误

如果遇到导入错误，确保：
1. Python 模块已正确构建
2. 模块路径在 Python 搜索路径中
3. 所有依赖库（dng.dll）在系统路径中

### 构建错误

如果构建失败：
1. 确保安装了 Python 开发头文件
2. 检查 CMake 是否找到了 Python
3. 确保 C++14 编译器可用

## 许可证

本项目基于 Adobe DNG SDK，请遵守相应的许可证要求。

