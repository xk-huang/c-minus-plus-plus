# c-minus-plus-plus

C Minus Plus Plus (c-++), a C minus compiler with more C features.

A course project for "BNU Compiler Theory Course 2019".

# English README

## 1. Environment and Usage

### 1.1 Environment

- System & IDE

  ```yaml
  macOS Mojave: 10.14.6
  Visual Studio Code: 1.41.0
  ```

- Environment

  ```yaml
  GNU Make: 3.81
  gcc-9 (Homebrew GCC 9.2.0): 9.2.0
  ```

- `encoding=utf-8`

### 1.2 Usage

1. Compiler Rebuilding:

  ```shell
  make clean
  make
  ```

2. Compiler Usage:

  ```shell
  ./compiler < [input file path] > [output file path]
  #e.g.: ./parser < ./testfile/gcd.c > ./testout/gcd.out
  ```

3. Run `test.sh` script to test the `compiler`. The inputs are in `./testfile/`, while the outputs are in `./testout/`:

  ```shell
  sh ./test.sh
  ```

# 中文版

C Minus Plus Plus (c-++), 又一个 C minus 编译器（添加更多 C 特性）。

北京师范大学“编译原理”课程项目 (2019)。

## 1. 开发环境及运行方法

### 1.1 开发环境信息

- 系统环境及开发 IDE

  ```yaml
  macOS Mojave: 10.14.6
  Visual Studio Code: 1.41.0
  ```

- 环境依赖

  ```yaml
  GNU Make: 3.81
  gcc-9 (Homebrew GCC 9.2.0): 9.2.0
  ```

- 编码：`encoding=utf-8`

### 1.2 运行方法

1. 编译 `c-++` 编译器，忽略 `warning`，产生 `compiler` 可执行文件：

  ```shell
  make clean
  make
  ```

2. `compiler` 使用标准输入输出，如果希望对文件读写，请使用重定向：

  ```shell
  ./compiler < [input file path] > [output file path]
  #e.g.: ./parser < ./testfile/gcd.c > ./testout/gcd.out
  ```

3. 运行 `test.sh` 来测试编译器，输入文件在 `./testfile/` 中，输出文件在 `./testout/` 中：

  ```shell
  sh ./test.sh
  ```
