using Newtonsoft.Json;

public class Demo
{
    static void Main(string[] args)
    {
        var path = @"C:\UnityLab\Github\PaddleOCR-json_v.1.3.0\PaddleOCR-json.exe";
        var imgPath = @"C:\Users\28904\Desktop\test.png";

        var ocr = new PPOCR_api(path);
        ocr.ParseImg(imgPath);

        ocr.Dispose();
        Console.ReadLine();
    }
}