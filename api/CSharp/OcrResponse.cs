public class OcrResponse
{
    public int code;
    public OcrEntry[] data;
    public string msg;
    public string hotUpdate;

    public override string ToString()
    {
        return "OcrResponse{" +
                "code=" + code +
                ", data=" + string.Join(", ", Array.ConvertAll(data, x => x.ToString())) +
                ", msg='" + msg + '\'' +
                ", hotUpdate='" + hotUpdate + '\'' +
                '}';
    }

    public OcrResponse()
    {
        data = new OcrEntry[0];
        msg = "";
        hotUpdate = "";
    }

    public OcrResponse(int code, string msg)
    {
        this.code = code;
        this.msg = msg;
        data = new OcrEntry[0];
        hotUpdate = "";
    }
}

public class OcrEntry
{
    public string text;
    public int[][] box;
    public double score;

    public override string ToString()
    {
        return "OcrEntry{" +
                "text='" + text + '\'' +
                ", box=" + string.Join(", ", Array.ConvertAll(box, x => "[" + string.Join(", ", x) + "]")) +
                ", score=" + score +
                '}';
    }
}