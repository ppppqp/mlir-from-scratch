# Mini Compiler

按照 [课程大纲](COURSE_OUTLINE.md) 用 C++20 手写一个教学用 AI 编译器后端。

当前进度：第 1 课第 3 节，已有 `mini_ir` 库与 `mini-opt` 命令行入口。

```bash
cmake -S . -B build
cmake --build build
./build/mini-opt --version
```

课程设计记录见 [docs/lesson01](docs/lesson01)。
