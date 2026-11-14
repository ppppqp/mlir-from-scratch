# Tests

`ir_use_def.cpp` 是第 2 课第 1 节的验收测试。骨架阶段它会因 `TODO` 抛出异常而失败；补全 `Value` 与 `Operation` 后应通过。

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

第 3 课再加入文本 IR 的解析、打印和诊断测试。

`ir_containers.cpp` 是第 2 课第 2 节的验收测试。骨架阶段它会因 `TODO` 失败；补全 Operation → Region → Block 嵌套、ModuleOp/FuncOp 包装后应通过，包括作用域结束时的逆序销毁。

`ir_rewrite.cpp` 是第 2 课第 3 节的验收测试。它检查仍有使用者的操作不能删除、RAUW 后 use 数正确、指定位置插入，以及嵌套 Operation 的先序遍历。骨架阶段会因 `TODO` 失败。

`ir_text.cpp` 是第 3 课第 2 节的验收测试。它检查函数属性、文本解析及稳定打印。骨架阶段会因 `TODO: Operation::setAttribute` 失败。
