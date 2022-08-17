<#
PowerShell 调用Umi-OCR的demo

不完善，当前存在问题：
1. 处理OCR结果的业务函数 OcrFlow 不运行在主窗口中，所以用Windows PowerShell运行代码似乎不会显示它输出的内容，只有在vscode中运行才可以看到。
2. 编码转换问题，得到的 $ocrStr 的内容是utf-8但形式是gbk，因此表现为乱码，要重新编码为utf-8才行。
3. 任务结束退出程序时似乎存在问题，OCR识别器线程能被正常关闭，而powershell进程未结束，exit不起作用。（强制杀死本线程$pid？）
#>

# 批量处理的图片，数组，支持中文
$inPutImgPaths = "D:\测试1.png", "D:\测试2.png", "D:\测试3.png"
# 当前下标，全局变量
$global:nowIndex = 0


# 业务函数，处理一轮OCR的结果。
# $ocrStr      : 上一轮识图的返回结果，一般为json字符串。第一轮前为空。
# $nextImgPath : 下一轮识图的图片路径，需要在函数结束前改写它。若不改动或置为空字符串，则本轮退出进程。
function OcrFlow([string]$ocrStr, [REF]$nextImgPath) {
    # TODO : 处理返回的字符串
    if ($ocrStr) {
        # TODO : 编码转换，gbk转utf-8
        Write-Host "第 $nowIndex 轮结果为：【$ocrStr】"
    }

    # TODO : 设置新图片路径
    if ($global:nowIndex -lt $inPutImgPaths.Length) {
        $nextImgPath.Value = $inPutImgPaths[$global:nowIndex]
    }
    $global:nowIndex += 1
}

function OcrInit {
    Write-Host "OCR进程初始化。"
    # 设置进程启动信息
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    # 识别器路径及工作路径（exe所在目录）
    # TODO : 修改路径
    $psi.FileName = "D:\MyCode\PythonCode\Umi-OCR\PaddleOCR-json\PaddleOCR_json.exe"
    $psi.WorkingDirectory = "D:\MyCode\PythonCode\Umi-OCR\PaddleOCR-json"
    # 进程重定向输入和输出
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.CreateNoWindow = $true
    $psi.UseShellExecute = $false
    $global:process = New-Object System.Diagnostics.Process # 进程对象为全局变量
    $process.StartInfo = $psi
    # 给System.Diagnostics.Process添加OutputDataReceived事件的订阅
    Register-ObjectEvent -InputObject $process -EventName OutputDataReceived -action {
        $nextImgPath = "" # 记录下一轮的图片路径
        $ocrStr = $Event.SourceEventArgs.Data
        if (!$ocrStr) { 
            Write-Host "第 $nowIndex 轮为空"
            return
        }
        if ($ocrStr -eq "Active code page: 65001") {
            $ocrStr = "" # 第零轮，返回值为空
        }
        # 向业务函数传递识图结果，并获取下一轮图片路径
        OcrFlow $ocrStr ([REF]$nextImgPath)
        # ===========================================
        Write-Host "第 $nowIndex 轮获取新图片地址：【$nextImgPath】"
        if (!$nextImgPath) {
            # 路径为空，结束进程
            $process.Kill()
            Write-Host "结束OCR进程。 $process"
            # TODO : 收尾，退出程序
            exit # 未退出？
        }
        # 向OCR进程写入下一轮图片路径
        # $Event输入自动化变量，包含了 Register-ObjectEvent 命令的Action参数中的上下文，尤其是Sender和Args。
        # .Sender默认是Object对象，需要转换成Process对象。
        $p = [System.Diagnostics.Process]$Event.Sender
        $p.StandardInput.WriteLine($nextImgPath);
    }
    $process.Start()
    $process.BeginOutputReadLine()
    Write-Host "OCR启动完毕。进程对象：$process"
}
OcrInit
