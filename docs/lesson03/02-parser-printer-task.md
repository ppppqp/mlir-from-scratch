# 第 3 课第 2 节：解析与打印任务

## 实现顺序

1. **属性存储**：完成 `Operation::setAttribute` 与 `findAttribute`。`func.func` 的 `sym_name` 是字符串，`function_type` 是 `FunctionType{inputs, results}`；`func.func` 的 SSA `resultCount()` 仍为 0。
2. **词法与类型**：识别标识符、`@`/`%` 名称、字符串操作名、整数维度、标点、`->`、空白和 `//` 行注释；解析 `f32`、一维和二维静态张量类型。
3. **构造 IR**：`parseModule` 创建 `builtin.module → Region → Block → func.func → Region → Block`；函数参数是 block arguments。每个函数维护一张 `%名称 → Value*` 表，遇到定义时插入、遇到使用时查找。计算操作的类型从 generic operation 的 `: (...) -> ...` 读取。
4. **稳定打印**：`printModule` 从 Operation 树及属性恢复函数名、签名和操作；给参数、操作结果分配稳定的 SSA 名，保证每次打印一致，打印结果可再次解析。

## 验收

运行 `ctest --test-dir build --output-on-failure`。新增 `ir-text` 测试检查属性、嵌套结构、use-def 链和 `parse → print → parse → print` 的稳定性。SSA 名允许从 `%A/%sum` 规范化为 `%arg0/%0`；函数名、类型和计算关系必须保留。详细的错误定位与非法输入诊断留到第 3 课第 3 节。
