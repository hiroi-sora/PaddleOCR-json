public class OcrResponse
{
    public int code;
    public OcrEntry[] data;
    public string msg;
    public string hotUpdate;
}

public class OcrEntry
{
    public string text;
    public int[][] box;
    public double score;
}