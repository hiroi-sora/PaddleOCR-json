public class OcrRequest
{
    //图片路径，或传入clipboard指定剪贴板识别
    public string img_path;
    //图片经过base64编码的字符串。
    public string image_base64;

    public void SetClipBoard()
    {
        img_path = "clipboard";
    }
}
