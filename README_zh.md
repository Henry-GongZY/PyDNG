# PyDNG — Adobe DNG SDK 的 Python 绑定

[English](./README.md)


本项目为 Adobe DNG SDK 提供基础的 Python 绑定，使您能够在 Python 中读取和写入 DNG（数字负片）文件。

## 特性

- 读取 DNG 文件
- 写入 DNG 文件
- 提取元数据（EXIF、相机信息等）
- 访问图像数据（Stage1 和 Stage3）
- 从 NumPy 数组设置图像数据
- 读取和写入基准曝光（Baseline Exposure）
- 原生支持 NumPy

## 仓库布局

- `src/pydng/` — 可安装的 Python 包（包含 `__init__.py`、类型存根、`py.typed`）。
- `bindings/` — 用于 pybind11 扩展（`_native`）和 `dng_validate` 的 C++ 层：
  - `include/` — 头文件（`dng.h`、`utils.h`、`pch.h`）
  - `src/` — `pydng_bindings.cpp`、`dng.cpp`
  - `main.cpp` — `dng_validate` 工具的入口点。
- `extern/` — 引入的 SDK 和第三方代码（DNG SDK、XMP、libjxl、pybind11 等）。

### CI 打包

在推送（Push）和拉取请求（PR）时，[GitHub Actions](.github/workflows/build.yml) 使用两阶段流水线：
1.  **阶段 1**：为 Linux 和 Windows 构建核心 `dng` 共享库。
2.  **阶段 2**：使用 `cibuildwheel` 为所有兼容版本（Python 3.8 到 3.12）构建 Python wheel，并使用预构建的核心库。

这确保了高效的构建时间并提供了广泛的兼容性。

## 快速开始

### 使用 pip 安装（推荐）

最简单的方法是一步安装：

```bash
# 从项目根目录安装
pip install .

# 可编辑安装（开发模式）
pip install -e .

# 从 Git 仓库安装
pip install git+https://github.com/yourusername/PyDNG.git
```

pip 会自动拉取构建依赖并为您驱动 CMake 构建。

### 手动构建

如果您需要对配置和编译进行完全控制，请使用手动 CMake 工作流。

## 环境要求

- CMake 3.15 或更高版本
- Python 3.8 或更高版本（包括您所针对的解释器的开发头文件）
- 支持 C++14 的编译器（Linux 上为 GCC 4.9+，macOS 上为 Clang 3.4+，Windows 上为 MSVC 2015+）
- pybind11（通过 `extern/pybind11` git 子模块，如果缺失则会自动获取）

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

构建 `dng_validate` 命令行工具以验证您的 C++ 更改：

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
import pydng
import numpy as np

# 从路径加载（失败时抛出 RuntimeError）
dng = pydng.Dng("input.dng", ignore_enhanced=False)

meta = dng.get_meta()
print(f"相机: {meta.make} {meta.model}")
print(f"图像尺寸: {meta.width} x {meta.height}")
print(f"ISO: {meta.iso}")
print(f"曝光时间: {meta.exposure_time} s")

data = dng.get_data(enhanced=False)
numpy_array = data.to_numpy()
print(f"图像形状: {numpy_array.shape}")
```

如果您更喜欢通过检查 `ErrorCode` 而不是处理异常，仍然可以使用 `dng = pydng.Dng()` 配合 `dng.read(path)`。

### 写入 DNG

```python
import pydng
import numpy as np

height, width, channels = 1000, 1500, 3
image_data = np.random.randint(0, 65535, size=(height, width, channels), dtype=np.uint16)

dng = pydng.Dng()

# 3 = ttShort (16位无符号)
dng.set_data(image_data, 3, enhanced=False)

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

error_code = dng.write("output.dng")
```

## API 参考

### `Dng` 类

读取和写入 DNG 文件的主要入口点。

#### 构造函数

- `Dng()` — 空对象；使用 `read()` 加载文件。
- `Dng(path: str, ignore_enhanced: bool = False)` — 立即加载 `path`；失败时抛出 `RuntimeError`（行为与 `read()` 返回非 `NONE` 代码一致）。

#### 方法

- `get_bayer_pattern() -> str`  
  对于 2×2 矩形 RGB CFA，返回 `"RGGB"`、`"GRBG"`、`"BGGR"` 或 `"GBRG"`（行优先平铺）；否则返回空字符串 `""`。

- `set_bayer_pattern(pattern: str) -> None`  
  设置 2×2 拜耳相位；`pattern` 必须是上述四个字符串之一（不区分大小写）。需要 3 平面 RGB CFA（或未初始化的马赛克，可通过 `SetRGB()` 初始化）。

- `read(path: str, ignore_enhanced: bool = False) -> ErrorCode`  
  从磁盘加载 DNG（返回代码；出错时不抛出异常）。

- `write(path: str) -> ErrorCode`  
  将 DNG 保存到磁盘。

- `get_data(enhanced: bool = False) -> DngData`  
  返回图像数据。`enhanced=True` 选择 Stage3；`False` 选择 Stage1。

- `set_data(data: np.ndarray, pixel_type: int, enhanced: bool = False) -> None`  
  设置图像数据。`data` 的形状为 `(height, width, channels)`。  
  `pixel_type`：数值代码（`1` = ttByte, `3` = ttShort, `8` = ttSShort, `4` = ttLong）。

- `get_meta() -> DngMeta`  
  返回元数据。

- `set_meta(meta: DngMeta) -> None`  
  应用元数据。

- `get_baseline_exposure() -> float`  
  基准曝光值。

- `set_baseline_exposure(exposure: float) -> None`  
  设置基准曝光。

- `get_white_balance() -> List[float]`  
  获取白平衡中性向量（例如 `[r, g, b]` 增益）。

- `set_white_balance(wb: List[float]) -> None`  
  设置白平衡中性向量。

### `DngMeta` 类

Dng 文件的元数据。

#### 字段

- `make`, `model`：相机制造商和型号
- `software`：软件字符串
- `artist`, `copyright`：作者和版权
- `width`, `height`：图像尺寸
- `raw_width`, `raw_height`：原始尺寸
- `exposure_time`：曝光时间（秒）
- `f_number`：光圈
- `focal_length`：焦距（mm）
- `iso`：感光度
- `focal_length_35mm`：等效 35mm 焦距
- `date_time`, `date_time_original`：时间戳
- `is_monochrome`：黑白标志
- `color_planes`, `color_space`：色彩布局和空间

### `DngData` 类

由 `get_data()` 返回的图像缓冲区。

#### 字段

- `width`, `height`, `channels`：布局
- `pixel_type`：内部类型代码
- `top`, `left`：有效区域偏移

#### 方法

- `to_numpy() -> np.ndarray`  
  导出为 NumPy 数组。

### 常量 — `ErrorCode`

- `NONE`：成功
- `READ_FILE`：读取失败
- `WRITE_FILE`：写入失败
- `BAD_FORMAT`：格式无效
- `UNKNOWN`：其他错误

像素类型代码（另请参阅 [PIXEL_TYPES.md](PIXEL_TYPES.md)）：

- `1` — ttByte (8位无符号)
- `3` — ttShort (16位无符号)
- `8` — ttSShort (16位有符号)
- `4` — ttLong (32位无符号)

## 示例

请参阅 `examples/` 目录：

- `example_read_dng.py` — 加载 DNG 并打印信息
- `example_write_dng.py` — 构建并写入 DNG

## 注意事项

1. **内存** — `DngData` 的生命周期与转换为 NumPy 数组的操作挂钩；请勿尝试手动释放底层指针。
2. **像素类型** — 在 `set_data()` 中选择 `pixel_type` 时，请确保它与数组的 dtype 和布局匹配。
3. **布局** — 图像数组预期为 `(height, width, channels)`。
4. **Windows 路径** — 在需要的地方，路径会使用适当的宽字符（Wide-character） API 进行处理。

## 故障排除

### 导入错误

1. 确认扩展模块已成功构建。
2. 确保构建输出位于 `PYTHONPATH` 中或已安装到 site-packages 中。
3. 在 Windows 上，必须能够找到原生依赖项（`dng.dll` 及相关文件），它们应与 `.pyd` 文件处于同一文件夹或位于 `PATH` 中。

### 构建失败

1. 为您的解释器安装 Python 开发包（头文件和库）。
2. 验证 CMake 是否找到了预期的 Python（`Python3_ROOT`、`CMAKE_PREFIX_PATH` 等）。
3. 确认您拥有可用的 C++14 工具链。

## 许可证

本项目基于 Adobe DNG SDK 构建；使用和再分发必须遵守适用于该 SDK 及本仓库的 Adobe 许可证条款。
