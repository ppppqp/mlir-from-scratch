# Mini Compiler

按照 [课程大纲](COURSE_OUTLINE.md) 用 C++20 手写一个教学用 AI 编译器后端。

当前进度：第 1 课和第 2 课第 1 节已完成；第 2 课第 2 节的 Operation → Region → Block 骨架和验收测试已建立，核心 `TODO` 待实现。

```bash
cmake -S . -B build
cmake --build build
./build/mini-opt --version
```

课程设计记录见 [docs/lesson01](docs/lesson01)。
