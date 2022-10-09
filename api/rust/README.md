## 安装

```powershell
cargo add paddlecor
```

或在 `Cargo.toml` 中加入：

```toml
[dependencies]
paddleocr = "0.2.2"
```

另外需要自行提供一份 release，本 crate 不包含可执行文件。

`0.2.2` 版本在 `v1.2.0`、`v1.2.1` 测试通过，前者不支持中文路径。

## 示例

```rust
fn main() {
    let mut p = paddleocr::Ppocr::new(std::path::PathBuf::from(
        "C:/.../PaddleOCR_json.exe", // PaddleOCR_json.exe 的路径
    ))
    .unwrap(); // 会检测是否出现 `OCR init completed.`，`Ok(x)` 说明初始化成功

    let now = std::time::Instant::now(); // 开始计算所需时间
    {
        // OCR 文件
        println!("{}", p.ocr("C:/.../test1.png").unwrap());
        println!("{}", p.ocr("C:/.../test2.png").unwrap());
        println!("{}", p.ocr("C:/.../test3.png").unwrap());
        println!("{}", p.ocr("C:/.../test4.png").unwrap());
        println!("{}", p.ocr("C:/.../test5.png").unwrap());

        // OCR 当前剪贴板
        println!("{}", p.ocr_clipboard().unwrap());
    }
    println!("Elapsed: {:.2?}", now.elapsed());

    // `struct Ppocr` 会自动在 `Drop` 时结束进程
}
```
