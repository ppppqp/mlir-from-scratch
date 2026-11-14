# 第 3 课第 1 节：文本 IR 语法

采用 MLIR 的文本语法和通用操作形式，但手写解析器只接受本页列出的子集。`builtin.module` 与 `func.func` 是容器操作；`mini.add`、`mini.relu`、`mini.matmul` 是本项目的自定义操作名。解析器无需支持 MLIR 的所有 dialect、属性、region 形式或自定义 assembly format。

## 完整示例

```mlir
module {
  func.func @compute(%A: tensor<4xf32>, %B: tensor<4xf32>) -> tensor<4xf32> {
    %sum = "mini.add"(%A, %B) : (tensor<4xf32>, tensor<4xf32>) -> tensor<4xf32>
    %out = "mini.relu"(%sum) : (tensor<4xf32>) -> tensor<4xf32>
    func.return %out : tensor<4xf32>
  }
}
```

这段文本已用本地 `mlir-opt --allow-unregistered-dialect` 验证可解析。`mini` dialect 尚未注册，因此调用 MLIR 工具时需要该开关；本项目的手写解析器只认下面的三个 `mini` 操作，不接受任意未注册操作。

## 当前接受的形式

1. 一个 `module { ... }`，其中包含一个或多个 `func.func`。函数名以 `@` 开头，在模块内唯一。
2. 函数写为 `func.func @name(%arg: type, ...) -> type { ... }`。第一版每个函数只有一个隐式入口 block；参数列表就是该 block 的参数。函数体以一条 `func.return %value : type` 结束。
3. 计算操作统一采用 MLIR **generic operation** 形式：`%result = "mini.op"(%operand, ...) : (operand-type, ...) -> result-type`。第一版每个计算操作恰好有一个结果，不支持属性、显式 region、多结果或后继 block。
4. 类型只接受 `f32`、`tensor<Lxf32>` 和 `tensor<MxNxf32>`；维度是正整数且在编译期已知。算子各自的合法形状仍按 [最小语言规格](../lesson01/02-ir-scope.md) 检查。
5. SSA 名以 `%` 开头，在单个函数内唯一。使用一个值之前必须已定义它；函数参数在函数体开始时已定义。允许空白和 `//` 行注释。

## 三个计算操作

| 操作名 | 输入个数 | 类型规则 |
| --- | ---: | --- |
| `"mini.add"` | 2 | 两个相同的一维张量，结果与输入同类型 |
| `"mini.relu"` | 1 | 一维张量，结果与输入同类型 |
| `"mini.matmul"` | 2 | `tensor<MxKxf32>` 与 `tensor<KxNxf32>`，结果为 `tensor<MxNxf32>` |

冒号后的函数类型是显式声明：解析器先读取操作数的声明类型和结果类型，再用 SSA 表中已记录的实际类型逐项比对；随后运行算子形状校验。例如 `%sum` 的结果类型来自 `-> tensor<4xf32>`，而 `%A`、`%B` 的实际类型来自函数参数声明。

## 当前明确不支持

动态维度 `?`、多 block、分支、控制流、属性字典、location、通用操作中的嵌套 region、多个结果、自定义简写形式，以及未列出的 dialect 操作。实现这些语法并非本月课程的前置条件。
