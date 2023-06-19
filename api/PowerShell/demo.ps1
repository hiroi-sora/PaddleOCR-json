Import-Module -Force ".\PPOCR_api.ps1"  #修改对应路径

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
$exePath = Convert-Path ".\PaddleOCR-json_v.1.3.0_alpha.2\PaddleOCR-json.exe"   #修改对应exe路径
$arg = ""  #参数其实在API内没处理
$ocr = [PPOCR]::new($exePath,$arg)

while (1) {
    $选择 = Read-Host "1:图片路径`n2:剪贴板`n3:图片Base64`n4:图片Byte`n其他则退出"
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
            $getObj = $ocr.runClipboard()
            Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n$($getObj.'data'|Out-String)`n"
        }
        3{
            $path = read-host "请输入图片路径，自动转换BASE64" 
            $imgBase64 = [convert]::ToBase64String([System.IO.FIle]::ReadAllBytes($path))
            if ($imgBase64) {
                $getObj = $ocr.runBase64($imgBase64)
                Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n$($getObj.'data'|Out-String)`n"
            }
        }
        4{
            $path = read-host "请输入图片路径，自动转换Byte"
            $imgByte = [System.IO.FIle]::ReadAllBytes($path)
            if ($imgByte) {
                $getObj = $ocr.runByte($imgByte)
                Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n$($getObj.'data'|Out-String)`n"
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
