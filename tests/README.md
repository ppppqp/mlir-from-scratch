# Tests

`ir_use_def.cpp` 是第 2 课第 1 节的验收测试。骨架阶段它会因 `TODO` 抛出异常而失败；补全 `Value` 与 `Operation` 后应通过。

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

第 3 课再加入文本 IR 的解析、打印和诊断测试。
