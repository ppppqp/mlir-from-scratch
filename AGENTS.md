1. 这个项目用来学习编译器知识。希望使用C++手写一个类LLVM/MLIR的AI编译器后端，能够完成从IR逐步降级到ptx的全过程。
2. 希望能参照MLIR的标准学习并手写简化版的optimization pass/analysis pass，比如高层的linag，tiling，bufferization，vectorization等等。目的是通过一个个小任务深入理解其中的原理和细节。可以设计一个IR subset，保证这个项目可以在一个月内完成，避免太大的scope。可以适当增加一些有亮点/非常规的优化pass，学习针对高性能GPU编译器的特殊考虑。
3. 你可以参照的仓库有：/home/qiping-pan/Documents/workspace/llvm/llvm-project和/home/qiping-pan/Documents/workspace/iree
4. 使用C++来构建项目。这样还可以顺便学习C++知识。
5. 可以手写类MLIR的编译框架（比如pipeline的嵌套），这样还可以顺便学习MLIR的知识。