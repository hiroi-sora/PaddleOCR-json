using System.Diagnostics;
using Newtonsoft.Json;


public class Ocr
{
    private string _exePath;
    private Process _process;
    private bool IsActive => !_process.HasExited;

    public Ocr(string path)
    {
        this._exePath = path;

        var name = this._exePath.Substring(this._exePath.LastIndexOf('\\') + 1);
        var dir = this._exePath.Substring(0, this._exePath.LastIndexOf('\\'));
        System.IO.Directory.SetCurrentDirectory(dir);

        _process = new Process();
        _process.StartInfo.UseShellExecute = false;
        _process.StartInfo.RedirectStandardOutput = true;//输出参数设定
        _process.StartInfo.RedirectStandardInput = true;//输出参数设定
        _process.StartInfo.CreateNoWindow = true;
        _process.StartInfo.FileName = name;


        _process.Start();
        while (true)
        {
            var strOut = _process.StandardOutput.ReadLine();
            if (strOut == "OCR init completed.")
            {
                break;
            }
        }

        Console.WriteLine("OCR init completed.");
    }

    ~Ocr()
    {
        _process.Kill();
    }


    // 分析单张图片
    public void ParseImgOnce(string imgPath)
    {
        if (!IsActive)
        {
            throw new Exception("OCR process is not active.");
        }


    }

    public void Dispose()
    {
        _process.Kill();
    }

    private OcrResponse SendJsonToOcr(OcrRequest request)
    {
        if (!IsActive)
        {
            throw new Exception("OCR process is not active.");
        }

        var setting = new JsonSerializerSettings
        {
            StringEscapeHandling = StringEscapeHandling.EscapeNonAscii
        };

        // 把json启用ASCII转义
        var json = JsonConvert.SerializeObject(request, setting);

        // 写入引擎进程的stdin流
        _process.StandardInput.WriteLine(json);
        _process.StandardInput.WriteLine("\r\n");
        _process.StandardInput.Flush();

        var resp = _process.StandardOutput.ReadLine();
        var ocrResp = JsonConvert.DeserializeObject<OcrResponse>(resp);

        return ocrResp;
    }
}
