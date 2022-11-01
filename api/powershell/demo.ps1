Import-Module -Force D:\…………\PPOCR_api.ps1

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
$ocr = [PPOCR]::new("D:\…………\PaddleOCR-json\PaddleOCR_json.exe")

# 识别图片，传入图片路径
while (1) {
    $imgPath = read-host "请输入图片路径，退出直接回车" 
    if ($imgPath) {
        $getObj = $ocr.run($imgPath)
        Write-Host "图片识别完毕，状态码：$($getObj.code) 结果：`n$($getObj.data | Out-String)`n"
    }
    else {
        break
    }
}

$ocr.stop()  # 结束子进程。
Write-Host "程序结束。"
