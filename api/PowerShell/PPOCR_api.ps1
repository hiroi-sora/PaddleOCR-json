# 调用 PaddleOCR-json.exe 的 PowerShell Api
# 项目主页：
# https://github.com/hiroi-sora/PaddleOCR-json

###########################################################################
function asc($param){ 
##用来转换中文至\uxxxx的函数
    $rtn = ''
    $list = $param -split ''
    foreach ($char in $list){
        if($char -ne ''){
            if([int]([char]$char) -gt 32 -and [int]([char]$char) -lt 127){
                $rtn  += $char
            }
            else{
                $rtn  += ("\u" + ("{0:x}" -f [int]([char]$char)))
            }
        }
    }
    return $rtn 
}
###########################################################################
class PPOCR {
    [System.Object]$process # 子进程对象
    [int] $runFlag = 0 # 运行标志。0正在初始化，1正常运行中
    [System.Diagnostics.Process] $stdSender # 保存子进程stdout管道
    [string] $imgJson # 缓存图片识别结果json字符串
    [string] $processID # OCR子进程id，用于组成标识符
    [string] $eventInit # OCR初始化完成的事件标识符，启动时定义
    [string] $eventJson # 一次取得json完成的事件标识符，传入图片时重新定义

    PPOCR( [string]$exePath,[string]$arg ) {
        # 初始化识别器。
        # :exePath: 识别器`PaddleOCR_json.exe`的路径。
        $WorkingDirectory = Split-Path -Path $exePath # 工作目录为父目录
        # 初始化进程信息，重定向输入和输出
        $psi = New-Object System.Diagnostics.ProcessStartInfo
        $psi.FileName = $exePath
        $psi.WorkingDirectory = $WorkingDirectory
        $psi.Arguments = $arg
        $psi.RedirectStandardInput = $true
        $psi.RedirectStandardOutput = $true
        $psi.CreateNoWindow = $true
        $psi.UseShellExecute = $false
        # 初始化进程对象
        $this.process = New-Object System.Diagnostics.Process
        $this.process.StartInfo = $psi
        # 给进程对象添加OutputDataReceived（得到进程输出内容）事件的订阅。通过-MessageData将$this传入-action作用域。
        Register-ObjectEvent -InputObject $this.process -EventName OutputDataReceived -action $this.getStdout -MessageData $this
        # 启动进程
        $this.process.Start()
        $this.process.BeginOutputReadLine()
        # 等待初始化完成事件
        $this.processID = $this.process.Id # 记录进程号
        $nowTime = Get-Date -Format "HHmmssffff"
        $this.eventInit = "OCRinit" + $this.processID + $nowTime # 生成事件标识符
        # Write-Host "初始化OCR等待中，事件标识符为"($this.eventInit)
        Wait-Event -SourceIdentifier $this.eventInit # 阻塞，等待初始化完成事件
        Write-Host "初始化OCR成功，进程号为"($this.process.Id)
    }

    # 接收stdout数据的action
    [ScriptBlock] $getStdout = {
        $this_ = $Event.MessageData # 获取所属对象
        $this_.stdSender = $Event.Sender # 将输入接口发送到调用对象
        $getData = $Event.SourceEventArgs.Data
        switch ( $this_.runFlag ) {
            # 初始化中，等待取得完成标志
            0 {
                if ( $getData.contains("OCR init completed.") ) {
                    $this_.runFlag = 1
                    New-Event -SourceIdentifier $this_.eventInit # 发送初始化完成事件
                }
                break
            }
            # 正常运行中
            1 {
                $this_.imgJson = $getData
                New-Event -SourceIdentifier $this_.eventJson # 发送取得json事件
                break
            }
        }
    }

    [PSCustomObject] runDict( [string]$writeDict ) {
        if ($this.stdSender) {
            $nowTime = Get-Date -Format "HHmmssffff"
            $this.eventJson = "OCRjson" + $this.processID + $nowTime # 更新事件标识符
            $this.stdSender.StandardInput.WriteLine($writeDict); # 向管道写入
            Wait-Event -SourceIdentifier $this.eventJson # 阻塞，等待取得json
            try {
                $getdict = $this.imgJson | ConvertFrom-Json
                return $getdict
            }
            catch {
                return @{code = 402; data = "识别器输出值反序列化JSON失败，疑似传入了不存在或无法识别的图片。异常信息：$($PSItem.ToString())原始内容：$($this.imgJson)" }
            }
        }
        else {
            # 输入流不存在，可能为未初始化完毕
            return @{ code = 400; data = "子进程输入流不存在" }
        }
    }


    # 识别图片
    [PSCustomObject] run( [string]$imgPath ) {
        # 对一张图片文字识别。
        # :imgPath: 图片路径。
        $writeDict = asc (@{ image_path = $imgPath } | ConvertTo-Json -Compress) #更新图片路径为json格式,asc函数替换中文
        return $this.runDict($writeDict); # 向管道写入图片路径
    }
    
    [PSCustomObject] runClipboard() {
        return $this.run("clipboard");
    }

    [PSCustomObject] runBase64( [string]$imgBase64 ) {
        $writeDict = @{ image_base64 = $imgBase64 } | ConvertTo-Json -Compress
        return $this.runDict($writeDict);
    }
    
    [PSCustomObject] runByte( $imgByte ) {
        $imgBase64 = [convert]::ToBase64String($imgByte)
        return $this.runBase64($imgBase64);
    }

    # 结束子进程
    [void] stop() {
        $this.stdSender.StandardInput.WriteLine('{"exit":""}')
        Write-Host "识别器进程结束。"
    }
}
