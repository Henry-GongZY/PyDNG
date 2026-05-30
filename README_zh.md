# PyDNG — Adobe DNG SDK 的 Python 绑定

[English](./README.md)


本项目为 Adobe DNG SDK 提供 Python 绑定，使您能够在 Python 中读取和写入 DNG（数字负片）文件。

## 特性

- 读取 DNG 文件
- 写入 DNG 文件
- 提取元数据（EXIF、相机信息等）
- 访问图像数据（Stage1 原始 和 Stage3 增强）
- 从 NumPy 数组设置图像数据
- 读取和写入基准曝光（Baseline Exposure）、白平衡
- 增益映射（Gain Map / 遮光校正）支持
- 拜耳图案检测与配置
- 原生支持 NumPy

## 仓库布局

- `src/dngpy/` — 可安装的 Python 包（`__init__.py`、类型存根、`py.typed`）。
- `bindings/` — 用于 pybind11 扩展（`_native`）和 `dng_validate` 的 C++ 层：
  - `include/` — 头文件（`dng.h`、`utils.h`、`pch.h`）
  - `src/` — `pydng_bindings.cpp`、`dng.cpp`
  - `main.cpp` — `dng_validate` 工具的入口点。
- `extern/` — 引入的 SDK 和第三方代码（DNG SDK、XMP、libjxl、pybind11、libjpeg、示例 DNG 文件）。
- `cmake/` — CMake 辅助脚本（源文件列表、libjxl 配置、复制脚本）。

### CI 打包

在[发布的 Release](.github/workflows/build.yml) 上，[GitHub Actions](.github/workflows/build.yml) 使用两阶段流水线：
1.  **阶段 1**：为 Linux（manylinux2014）、Windows（MSVC）和 macOS（arm64）构建核心 `dng` 共享库。
2.  **阶段 2**：使用 `cibuildwheel` 为所有兼容版本（Python 3.8+）构建 Python wheel，并使用预构建的核心库。

这确保了高效的构建时间并提供了广泛的兼容性。

## 快速开始

### 使用 pip 安装（推荐）

```bash
# 从项目根目录安装
pip install .

# 可编辑安装（开发模式）
pip install -e .

# 从 PyPI 安装（如果可用）
pip install dngpy
```

pip 会自动拉取构建依赖并为您驱动 CMake 构建。

### 手动构建

如果您需要对配置和编译进行完全控制，请使用手动 CMake 工作流。

## 环境要求

- CMake 3.15 或更高版本
- Python 3.8 或更高版本（包括您所针对的解释器的开发头文件）
- 支持 C++14 的编译器（Linux 上为 GCC 4.9+，macOS 上为 Clang 3.4+，Windows 上为 MSVC 2015+）
- pybind11（引入在 `extern/pybind11` 中；如缺失则会自动获取）

### 标准构建（推荐）

为您当前的 Python 环境一次性构建所有内容（核心库和 Python 扩展）：

```bash
pip install .
```

### 进阶：分离构建

如果您想先构建一次核心库，然后再构建绑定（类似于 CI 过程）：

#### 1. 构建核心库

```bash
mkdir build
cd build
cmake .. -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_DNG_VALIDATE=ON -DCMAKE_INSTALL_PREFIX=../install_dir
cmake --build . --target install --config Release
```

#### 2. 使用预构建的核心库构建 Python 绑定

```bash
cd ..
# 指向之前步骤中的 install_dir
pip install . --config-settings=cmake.args="-DBUILD_DNG_LIBRARY=OFF -DPREBUILT_DNG_PATH=./install_dir"
```

### 构建 dng_validate 进行 C++ 验证

```bash
mkdir build
cd build
cmake .. -DBUILD_DNG_VALIDATE=ON -DBUILD_PYTHON_BINDINGS=OFF
cmake --build . --config Release --target dng_validate
```

可执行文件将位于 `build` 目录中（Windows 上位于 `build/Release`）。

## 使用方法

### 基础示例 — 读取 DNG

```python
import dngpy
import numpy as np

# 从路径加载（失败时抛出 RuntimeError）
dng = dngpy.Dng("input.dng", ignore_enhanced=False)

meta = dng.get_meta()
print(f"相机: {meta.make} {meta.model}")
print(f"图像尺寸: {meta.width} x {meta.height}")
print(f"ISO: {meta.iso}")
print(f"曝光时间: {meta.exposure_time} s")

data = dng.get_data(enhanced=False)
numpy_array = data.to_numpy()
print(f"图像形状: {numpy_array.shape}")
```

如果您更喜欢通过检查 `ErrorCode` 而不是处理异常，也可以使用 `dng = dngpy.Dng()` 配合 `dng.read(path)`。

### 写入 DNG

```python
import dngpy
import numpy as np

height, width, channels = 1000, 1500, 3
image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)

dng = dngpy.Dng()

# pixel_type 可以是整数或字符串名：
#   "uint16" / 3 = ttShort（16位无符号）
#   "uint8"  / 1 = ttByte （8位无符号）
dng.set_data(image_data, "uint16", enhanced=False)

meta = dngpy.DngMeta()
meta.make = "My Camera"
meta.model = "Example"
meta.software = "dngpy"
meta.width = width
meta.height = height
meta.color_planes = channels
meta.iso = 100
meta.exposure_time = 1.0 / 60.0
meta.f_number = 2.8
meta.focal_length = 50.0

dng.set_meta(meta)
dng.set_baseline_exposure(0.5)
dng.set_white_balance([1.0, 1.0, 1.0])

error_code = dng.write("output.dng")
if error_code != dngpy.ErrorCode.NONE:
    print(f"写入失败，错误码: {error_code}")
```

## API 参考

### `Dng` 类

读取和写入 DNG 文件的主要入口点。

#### 构造函数

- `Dng()` — 空对象；使用 `read()` 加载或 `set_data()` 创建新图像。
- `Dng(path: str, ignore_enhanced: bool = False)` — 立即加载 `path`；失败时抛出 `RuntimeError`。

#### 方法

- `read(path: str, ignore_enhanced: bool = False) -> int`  
  从磁盘加载 DNG（返回错误码；出错时不抛出异常）。

- `write(path: str) -> int`  
  将 DNG 保存到磁盘。

- `get_data(enhanced: bool = False) -> DngData`  
  返回图像数据。`enhanced=True` 选择 Stage3；`False` 选择 Stage1（原始）。

- `set_data(data: np.ndarray, pixel_type: int | str, enhanced: bool = False) -> None`  
  设置图像数据。`data` 的形状为 `(height, width, channels)`。  
  `pixel_type` 接受数值代码或字符串名称（见下方像素类型表）。

- `get_meta() -> DngMeta`  
  返回合并后的元数据（EXIF + 图像信息 + 颜色信息）。

- `set_meta(meta: DngMeta) -> None`  
  应用元数据。

- `get_exif() -> DngMeta`  
  仅返回 EXIF 元数据。

- `get_image_info() -> DngMeta`  
  返回图像几何尺寸信息。

- `get_color_info() -> DngMeta`  
  返回色彩空间和平面信息。

- `get_baseline_exposure() -> float`  
  基准曝光值。

- `set_baseline_exposure(exposure: float) -> None`  
  设置基准曝光。

- `get_white_balance() -> list[float]`  
  获取白平衡中性向量（例如 `[r, g, b]` 增益）。

- `set_white_balance(wb: list[float]) -> None`  
  设置白平衡中性向量。

- `get_gainmap() -> DngGainMap | None`  
  获取增益映射（遮光校正），如果存在。

- `set_gainmap(map: DngGainMap) -> None`  
  设置增益映射。

- `get_bayer_pattern() -> str`  
  对于 2×2 矩形 RGB CFA，返回 `"RGGB"`、`"GRBG"`、`"BGGR"` 或 `"GBRG"`（行优先）；否则返回空字符串。

- `set_bayer_pattern(pattern: str) -> None`  
  设置 2×2 拜耳相位；`pattern` 必须是上述四个字符串之一（不区分大小写）。

### 像素类型

`set_data()` 支持以下类型（可使用整数代码或字符串名称）：

| 字符串         | 代码 | DNG 常量      | C 类型     |
|---------------|------|--------------|------------|
| `"uint8"`     | 1    | ttByte       | uint8_t    |
| `"uint16"`    | 3    | ttShort      | uint16_t   |
| `"int16"`     | 8    | ttSShort     | int16_t    |
| `"uint32"`    | 4    | ttLong       | uint32_t   |
| `"float32"`   | 11   | ttFloat      | float      |

### `DngMeta` 类

DNG 文件的元数据。

#### 字段

| 字段               | 类型   | 描述               |
|--------------------|--------|--------------------|
| `make`             | str    | 相机制造商          |
| `model`            | str    | 相机型号            |
| `software`         | str    | 软件字符串          |
| `artist`           | str    | 作者               |
| `copyright`        | str    | 版权信息            |
| `width`            | int    | 图像宽度（裁剪后）   |
| `height`           | int    | 图像高度（裁剪后）   |
| `raw_width`        | int    | 原始图像宽度         |
| `raw_height`       | int    | 原始图像高度         |
| `exposure_time`    | float  | 曝光时间（秒）       |
| `f_number`         | float  | 光圈值              |
| `focal_length`     | float  | 焦距（mm）          |
| `iso`              | int    | ISO 感光度          |
| `focal_length_35mm`| int    | 等效 35mm 焦距      |
| `date_time`        | str    | 拍摄时间戳          |
| `date_time_original`| str   | 原始拍摄时间戳       |
| `is_monochrome`    | bool   | 黑白标志            |
| `color_planes`     | int    | 色彩平面数           |
| `color_space`      | str    | 色彩空间名称         |

### `DngData` 类

由 `get_data()` 返回的图像缓冲区。

#### 字段

- `width`、`height`、`channels` — 布局
- `pixel_type` — 内部类型代码（见像素类型表）
- `top`、`left` — 有效区域偏移

#### 方法

- `to_numpy() -> np.ndarray` — 导出为 NumPy 数组。

### `DngGainMap` 类

遮光校正（增益映射），由 `get_gainmap()` 返回。

#### 字段

- `rows`、`cols`、`planes` — 网格尺寸
- `spacing_v`、`spacing_h` — 网格点间距
- `origin_v`、`origin_h` — 网格起点
- `data` — 增益值的 NumPy 数组

#### 方法

- `to_numpy() -> np.ndarray` — 将增益映射数据导出为 NumPy 数组。

### 常量 — `ErrorCode`

- `NONE = 0` — 成功
- `READ_FILE` — 读取失败
- `WRITE_FILE` — 写入失败
- `BAD_FORMAT` — 无效格式
- `UNKNOWN` — 其他错误

## 示例

请参阅 `examples/` 目录：

- `example_read_dng.py` — 加载 DNG 并打印元数据、图像数据和增益映射
- `example_write_dng.py` — 构建并写入合成 DNG

测试用的示例 DNG 文件位于 `extern/sample_files/`。

## 注意事项

1. **内存** — `DngData` 的生命周期与 NumPy 转换操作挂钩；请勿手动释放底层指针。
2. **像素类型** — 在 `set_data()` 中选择 `pixel_type` 时，请确保它与数组的 dtype 和布局匹配。类型不匹配可能导致无效的 DNG 文件。
3. **布局** — 图像数组预期为 `(height, width, channels)` 的 C-contiguous 顺序。
4. **Windows 路径** — 在需要的地方，路径会使用宽字符 API 进行处理。
5. **写入时的调用顺序** — 在其他设置方法（`set_baseline_exposure`、`set_white_balance` 等）之前先调用 `set_data()`，因为它负责初始化内部 negative。

## 故障排除

### 导入错误

1. 确认扩展模块已成功构建。
2. 验证 Python 版本是否与编译的 `.pyd`/`.so` 后缀匹配（例如 `cp313` = Python 3.13）。
3. 在 Windows 上，`dng.dll` 必须与 `_native.pyd` 处于同一文件夹（或在 `PATH` 中）。

### 构建失败

1. 为您的解释器安装 Python 开发包（头文件和库）。
2. 验证 CMake 是否找到了预期的 Python（`Python3_ROOT`、`CMAKE_PREFIX_PATH` 等）。
3. 确认您拥有可用的 C++14 工具链。
4. 在 Unix 上，`libjpeg` 和 `libjxl` 作为外部项目构建，需要 autotools / CMake。

## 许可证

本项目基于 Adobe DNG SDK 构建；使用和再分发必须遵守适用于该 SDK 及本仓库的 Adobe 许可证条款。
