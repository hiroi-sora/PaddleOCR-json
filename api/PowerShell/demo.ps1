Import-Module -Force ".\PPOCR_api.ps1"  #修改对应路径

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
$exePath = Convert-Path ".\PaddleOCR-json_v.1.3.0_alpha.2\PaddleOCR-json.exe"   #修改对应exe路径
$arg = ""  #参数其实在API内没处理
$ocr = [PPOCR]::new($exePath,$arg)

while (1) {
    $prompt_str = "1:图片路径`n2:图片Base64`n3:图片Byte`n"
    if ( $ocr.isClipboardEnabled() ) {
        $prompt_str += "4:剪贴板`n"
    }
    $prompt_str += "其他则退出"
    $选择 = Read-Host $prompt_str
    switch($选择){
        1{
            # 识别图片
            $imgPath = read-host "请输入图片路径"
            if ($imgPath) {
                $getObj = $ocr.run($imgPath)
                Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n$($getObj.'data'|Out-String)`n"
            }
        }
        2{
            $path = read-host "请输入图片路径，自动转换BASE64" 
            $imgBase64 = [convert]::ToBase64String([System.IO.FIle]::ReadAllBytes($path))
            if ($imgBase64) {
                $getObj = $ocr.runBase64($imgBase64)
                Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n$($getObj.'data'|Out-String)`n"
            }
        }
        3{
            $path = read-host "请输入图片路径，自动转换Byte"
            $imgByte = [System.IO.FIle]::ReadAllBytes($path)
            if ($imgByte) {
                $getObj = $ocr.runByte($imgByte)
                Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n$($getObj.'data'|Out-String)`n"
            }
        }
        # 以下示例默认禁用
        4{
            if ( $ocr.isClipboardEnabled() ) {
                $getObj = $ocr.runClipboard()
                Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n$($getObj.'data'|Out-String)`n"
            }
            else {
                $ocr.stop()  # 结束子进程。
                Write-Host "程序结束。"
                Exit
            }
        }
        Default
        {
            $ocr.stop()  # 结束子进程。
            Write-Host "程序结束。"
            Exit
        }
    }
}
