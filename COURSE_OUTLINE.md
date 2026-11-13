# 一个月手写 AI 编译器后端：交互式课程大纲

## 目标与边界

用 C++20 实现一个教学用多级 IR 编译器，将静态形状的 `f32` 张量程序逐步降级为 NVIDIA GPU 的 PTX 内核。课程以 **12 课、4 周、每周 3 课** 为节奏；每课完成一个能运行或能检查的增量。框架借鉴 MLIR 的 SSA、嵌套 operation、pass/analysis 和合法化思想，但代码由学员手写，不依赖 MLIR 库。

第一版只支持一维逐元素 `add`、`relu` 和二维 `matmul`；张量维度在编译期已知，一维长度是 4 的倍数，矩阵维度是 16 的倍数，缓冲区地址保证 16 字节对齐。输出是 device-side PTX，输入输出缓冲区由调用方提供。暂不实现动态形状、自动求导、卷积、跨 kernel 调度、CUDA host runtime、Tensor Core 和完整 MLIR 兼容性。矩阵乘法先实现每线程计算一个输出元素的标量 FMA 基线，再实现 shared memory 分块复用；`vector<4xf32>` 用于逐元素内核的连续访存。

建议的降级路径：

```text
tensor IR (add / relu / matmul)
  -> tiled tensor IR (tile / loop / fusion)
  -> buffer IR (alloc / load / store)
  -> vector + GPU IR (vector.load/store, block_id, thread_id, shared, barrier)
  -> PTX
```

课程以同一个例子贯穿：`relu(add(A, B))`。第二个例子是 `matmul(A, B)`，用来讨论归约、二维 tiling 和线程映射。每课按“小节 1 → 小节 2 → 小节 3 → 小节 4”推进：先解释并读少量参考实现，再做一个最小练习，检查 IR 或测试结果，确认理解后进入下一小节。学员可随时要求“继续第 N 课第 M 节”或提交当前代码、IR、报错，让下一步围绕实际结果调整。

## 贯穿课程的性能主线

优化必须有可关闭的基线实现、适用条件和可观察的收益。`--opt-level=0` 为每个原始计算操作生成一个朴素内核，`--opt-level=1` 开启融合、向量化与访存映射，`--opt-level=2` 再开启 matmul 的 shared memory 分块和归约循环优化。每个优化能用 `--disable-pass=<name>` 单独关闭，避免把多项收益混在一起。

| 优化 pass | 作用与适用条件 | 预期改善与代价 | 课堂验收 |
| --- | --- | --- | --- |
| `fuse-elementwise` | 单用户 `add -> relu` 合成一个 kernel | 少一次中间张量写回、读回及一次 launch；可能增加寄存器需求 | 比较 kernel 数、全局访存字节和延迟 |
| `vectorize-elementwise` | 16 字节对齐、长度为 4 的倍数时，每线程处理 4 个连续元素 | 减少访存指令和地址计算；单线程寄存器用量增加 | 比较 PTX 访存指令数、吞吐量 |
| `coalesce-thread-mapping` | 矩阵列 N 映射到相邻 lane，保证 B 和 C 的相邻地址 | 减少全局内存事务；A 的重复读取仍需后续处理 | 打印前 32 个 lane 的地址步长，比较延迟 |
| `promote-matmul-to-shared` | `16×16×16` 固定 tile，256 线程协作加载 A/B，计算前与覆盖 tile 前同步 | 一个 tile 阶段的全局加载量从朴素的约 `2×16³` 个元素降为 `2×16²` 个元素；增加 shared memory、同步和寄存器压力 | 比较全局 load 数、shared 用量与大矩阵延迟 |
| `unroll-k` | 对固定的 K tile 循环按 2 或 4 展开，并提取循环不变量 | 减少分支和重复地址计算；代码体积与寄存器压力可能升高 | 比较分支数、寄存器数与延迟，回退无收益的配置 |

表中的访存量是源码层面的上界估算，不等于实际 DRAM 事务；真实收益要用相同输入和设备上的 A/B 测量确认。若本机只有 NVIDIA 驱动而没有 CUDA 工具包，仍可先完成 IR 统计和 PTX 生成；有 `ptxas`、CUDA Driver API 时再加入汇编和内核计时。

## 第 1 周：建立可观察的编译器骨架

### 第 1 课：定规格和搭建工程

1. **画出端到端路径**：明确 tensor、tile、buffer、GPU、PTX 各层保留和丢弃的信息；为 `relu(add(A,B))` 写一份手工降级草图。
2. **固定最小语言与性能基线**：定义类型、静态形状、三个计算算子及非法输入；约定未优化版本的 kernel 数、访存次数和测量方法。
3. **搭建 C++20 工程**：建立 `include/`、`lib/`、`tools/`、`tests/`，用 CMake 生成一个可运行的 `mini-opt` 命令。
4. **交互检查点**：提交一页 IR 规格和一个能打印版本号的可执行文件；逐项回答“为何先限制形状”和“每一层为何存在”。

### 第 2 课：手写 SSA IR

1. **值和使用链**：实现 `Type`、`Value`、`Operation`、operand/result 与 use-def 关系，练习 C++ 所有权和稳定对象地址。
2. **容器和嵌套**：实现 `Module`、`Function`、`Block`，让 pass 能只遍历某个函数中的操作。
3. **替换与删除**：实现 `replaceAllUsesWith`、操作插入/删除和基本遍历；避免悬空引用。
4. **交互检查点**：在内存中构造 `relu(add(A,B))`，打印每个结果的使用次数，并演示一次安全替换。

### 第 3 课：文本 IR、校验器与诊断

1. **可读语法**：制定一套极小文本格式，包含函数签名、SSA 名、类型、形状和操作属性。
2. **解析与打印**：实现解析器及稳定打印；先覆盖课程所需语法，不追求通用 MLIR parser。
3. **校验和错误定位**：检查未定义值、重复定义、形状不匹配、非法算子和函数返回值。
4. **交互检查点**：要求 `parse(print(ir))` 与原 IR 等价；故意输入 3 个错误案例并观察明确诊断。

## 第 2 周：在张量层做语义明确的优化

### 第 4 课：嵌套 pass 与 analysis 框架

1. **Operation pass**：设计 `Pass` 接口以及 module/function 两级 pass manager；支持 `module(func(canonicalize))` 这样的嵌套 pipeline。
2. **只读 analysis**：实现按函数计算的 use-count/shape analysis，区分 analysis 与修改 IR 的 pass。
3. **缓存失效和失败传播**：改写后使相关 analysis 失效；pass 失败时停止后续 pipeline，保留诊断。
4. **交互检查点**：用 `--print-ir-after-all` 显示每一步的 IR，验证嵌套顺序、缓存命中和失效。

### 第 5 课：结构化张量操作与参考解释器

1. **语义建模**：为 `add`、`relu`、`matmul` 定义迭代域、索引关系和归约维度，理解类似 `linalg` 的结构化信息。
2. **目的地语义**：给输出张量引入显式 `outs`/init 概念，说明后续原地 bufferization 的依据。
3. **CPU 参考解释器**：按原始张量语义执行小输入，产生优化前后的数值基准。
4. **交互检查点**：给 `16×16 matmul` 输入稀疏数据，手算左上角元素和一个逐元素链，再与解释器输出比对。

### 第 6 课：规范化与生产者消费者融合

1. **重写规则**：实现恒等操作消除、死操作删除等简单 canonicalization；避免不终止的 rewrite。
2. **融合判定**：将单用户的 `add -> relu` 融为一个逐元素计算区域，检查中间值是否还有其他使用者。
3. **收益边界**：对多用户情形比较重算与中间结果物化，第一版采取明确的保守规则；给融合 pass 加独立开关。
4. **交互检查点**：打印融合前后 IR、kernel 数和中间张量读写量，用参考解释器验证结果；添加一个多用户反例。

## 第 3 周：逐步降低到 GPU 可执行语义

### 第 7 课：Tiling、循环与边界

1. **分块变换**：把逐元素迭代域和 `matmul` 的 M/N 维分成静态 tile，显式表示 tile 起点和范围。
2. **K 维分块与复用估算**：把 `matmul` 的归约分成长度 16 的阶段；计算每个阶段 A/B tile 的加载次数，为 shared memory 优化建立基线。
3. **索引与合法性**：推导局部到全局地址，检查输出覆盖、无重复写入和归约顺序；固定倍数形状简化边界处理。
4. **交互检查点**：手算一个输出地址和两种方案的加载次数；扩展解释器执行 tiled IR 并对比分块前后结果。

### 第 8 课：Bufferization 与别名分析

1. **从值到存储**：定义 `memref`、`alloc`、`load`、`store`、`copy`，解释 SSA 张量与可变缓冲区的差别。
2. **原地判定**：沿 use-def 链检查 `outs` 是否仍有旧值使用；安全时复用输入缓冲区，否则分配或拷贝。
3. **降级和清理**：把 tensor/tile 操作改写成 buffer 操作，删除死中间缓冲区；对输出参数禁止错误别名。
4. **交互检查点**：构造一个可原地和一个必须复制的例子，打印分配/拷贝计数；执行 buffer IR 并比较数值结果。

### 第 9 课：Vectorization 与连续访存

1. **向量 IR**：加入 `vector<4xf32>`、连续 `vector.load/store` 和逐 lane 算术操作。
2. **向量化规则**：对一维逐元素循环每次处理 4 个元素，映射到 4 个标量寄存器与 PTX 向量访存；只在静态倍数与满足对齐约束时应用。
3. **循环微优化**：为 matmul 的固定 K tile 加 `unroll-k` 与循环不变量提取；控制展开因子，观察代码体积和寄存器压力。
4. **交互检查点**：比较向量化前后的访存指令数、展开前后的分支数；扩展解释器执行 vector IR 并比较输出。

## 第 4 周：线程映射、PTX 与完整验收

### 第 10 课：GPU 映射、访存合并与 shared memory

1. **显式 GPU IR 与映射**：加入 `block_id`、`thread_id`、`block_dim`；逐元素内核每线程负责连续 4 个元素，matmul 每线程负责一个输出。
2. **合并访存分析**：把矩阵列 N 映射到相邻 lane，比较 A/B/C 的地址步长；对不连续映射给出分析报告。
3. **shared memory 提升**：为 matmul 加固定 `16×16×16` A/B tile、协作加载、`barrier` 和局部复用；校验同步点与 shared 用量。
4. **交互检查点**：手算一个 tile 阶段前 4 个 lane 的全局与 shared 地址；用小型 tile 模拟器比较朴素版和提升版的全局 load 数及输出。

### 第 11 课：合法化与 PTX 发射

1. **目标合法性**：列出 PTX 后端支持的 IR 操作、类型与寻址模式；未降低的 tensor/tile 操作必须报错。
2. **函数与寄存器**：发射 `.entry`、参数、寄存器声明、线程索引和地址计算；处理全局内存地址空间。
3. **算术、访存与同步**：发射逐元素 `add`/`max`、向量 `ld/st`、matmul 的 `fma.rn.f32`，以及 shared `ld/st` 和 `bar.sync`。
4. **交互检查点**：从两份文本 IR 生成朴素版与优化版 PTX，检查关键指令、同步位置和未合法化操作；有 `ptxas` 时再做汇编验证。

### 第 12 课：端到端测试、测量与复盘

1. **整条 pipeline**：把解析、校验、各层 pass 和 PTX 发射接到 `mini-opt`，支持 `--stop-after`/`--emit` 查看任一层。
2. **差分与负例**：逐一关闭优化 pass，对照参考解释器验证结果；覆盖形状不合法、错误别名、同步遗漏、未合法化操作等负例。
3. **效果测量与选择**：报告 kernel 数、全局访存量、shared 用量及代码大小；有 `ptxas` 时报告实际寄存器数，有 CUDA 运行条件时在固定数据规模上预热并重复计时，选择有收益的展开因子。
4. **交互检查点**：提交逐 pass A/B 表，展示 `relu(add(A,B))` 与 `matmul(A,B)` 的最终 PTX 和正确性；具备 GPU 计时条件时再报告实际延迟，否则只报告静态指标，不宣称加速。

## 每课交付与推进方式

每课保留一份小输入 IR、一份期望输出或检查规则、一个可运行命令，以及简短复盘。互动时一次只推进一个小节：先预测 IR 变化，再实现，再运行并解释结果。若预测与结果不符，先定位该层的不变量或 pass 的前置条件，再进入下一小节。第 3、6、9、12 课分别是阶段验收点；前三个验收点应能独立运行，即使还没有 PTX 后端。

## 参考仓库中的对应位置

- [MLIR Toy 教程](../llvm/llvm-project/mlir/docs/Tutorials/Toy/_index.md)：教学型 dialect、pass 和逐步降级。
- [MLIR Pass Infrastructure](../llvm/llvm-project/mlir/docs/PassManagement.md)：operation pass、analysis 缓存、嵌套 pipeline。
- [MLIR Linalg Dialect](../llvm/llvm-project/mlir/docs/Dialects/Linalg/_index.md)：结构化算子的迭代域、tiling、fusion、vectorization。
- [MLIR Bufferization](../llvm/llvm-project/mlir/docs/Bufferization.md)：目的地语义、别名判断、tensor 到 memref。
- [MLIR Dialect Conversion](../llvm/llvm-project/mlir/docs/DialectConversion.md)：合法操作、重写规则、部分与完全转换。
- [MLIR GPU Dialect](../llvm/llvm-project/mlir/docs/Dialects/GPU.md)：GPU 线程抽象与目标降级边界；本课程为控制规模直接发射 PTX。
- [IREE LLVMGPU Passes](../iree/compiler/src/iree/compiler/Codegen/LLVMGPU/Passes.cpp)：GPU 配置、分块、分发、向量化和目标 lowering 的阶段安排。
- [MLIR GPU Memory Promotion](../llvm/llvm-project/mlir/lib/Dialect/GPU/Transforms/MemoryPromotion.cpp)：协作拷贝、访存合并与 barrier 的参考实现。
