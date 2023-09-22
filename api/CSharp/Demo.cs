using Newtonsoft.Json;

public class Demo
{
    static void Main(string[] args)
    {
        var path = @"C:\UnityLab\Github\PaddleOCR-json_v.1.3.0\PaddleOCR-json.exe";




        var ocr = new Ocr(path);
        //ocr.ParseImgOnce();

        ocr.Dispose();
        Console.ReadLine();
    }
}