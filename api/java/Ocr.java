// from: https://github.com/jerrylususu/PaddleOCR-json-java-api


package org.example;

import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;

import java.io.*;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.Map;

enum OcrCode {
    @SerializedName("100")
    OK(100),
    @SerializedName("101")
    NO_TEXT(101),
    @SerializedName("200")
    ERR_IMG_PATH(200),
    @SerializedName("201")
    ERR_IMG_READ(201),
    @SerializedName("300")
    ERR_JSON_DUMP(300);

    private final int code;

    OcrCode(int code) {
        this.code = code;
    }

    public int getValue() {
        return code;
    }

    public static OcrCode fromValue(int value) {
        for (OcrCode code : OcrCode.values()) {
            if(value == code.getValue()) {
                return code;
            }
        }
        return null;
    }
}

class OcrEntry {
    String text;
    int[] box;
    double score;

    @Override
    public String toString() {
        return "RecognizedText{" +
                "text='" + text + '\'' +
                ", box=" + Arrays.toString(box) +
                ", score=" + score +
                '}';
    }
}

class OcrResponse {
    OcrCode code;
    OcrEntry[] data;
    String msg;
    String hotUpdate;

    @Override
    public String toString() {
        return "OcrResponse{" +
                "code=" + code +
                ", data=" + Arrays.toString(data) +
                ", msg='" + msg + '\'' +
                ", hotUpdate='" + hotUpdate + '\'' +
                '}';
    }

    public OcrResponse() {
    }

    public OcrResponse(OcrCode code, String msg) {
        this.code = code;
        this.msg = msg;
    }
}

public class Ocr implements AutoCloseable {
    Process p;
    BufferedReader reader;
    BufferedWriter writer;
    Gson gson;

    boolean ocrReady = false;

    public Ocr(File exePath, Map<String, Object> arguments) throws IOException {
        gson = new Gson();

        String commands = "";
        if (arguments != null) {
            for (Map.Entry<String, Object> entry : arguments.entrySet()) {
                String command = "--" + entry.getKey() + "=";
                if (entry.getValue() instanceof String) {
                    command += "'" + entry.getValue() + "'";
                } else {
                    command += entry.getValue().toString();
                }
                commands += ' ' + command;
            }
        }

        if (!commands.contains("use_system_pause")) {
            commands += ' ' + "--use_system_pause=0";
        }

        System.out.println("当前参数：" + commands);

        File workingDir = exePath.getParentFile();
        ProcessBuilder pb = new ProcessBuilder(exePath.toString(), commands);
        pb.directory(workingDir);
        pb.redirectErrorStream(true);
        p = pb.start();

        InputStream stdout = p.getInputStream();
        OutputStream stdin = p.getOutputStream();
        reader = new BufferedReader(new InputStreamReader(stdout, Charset.forName("UTF-8")));
        writer = new BufferedWriter(new OutputStreamWriter(stdin, Charset.forName("UTF-8")));

        String line = "";
        ocrReady = false;
        while (!ocrReady) {
            line = reader.readLine();
//            System.out.println(line);
            if (line.contains("OCR init completed")) {
                ocrReady = true;
            }
        }

        System.out.println("初始化OCR成功");
    }

    public OcrResponse runOcr(File imgFile) throws IOException {
        if (!p.isAlive()) {
            throw new RuntimeException("OCR进程已经退出");
        }
        writer.write(imgFile.toString());
        writer.write("\r\n");
        writer.flush();
        String resp = reader.readLine();
        System.out.println(resp);

        Map rawJsonObj = gson.fromJson(resp, Map.class);
        if (rawJsonObj.get("data") instanceof String) {
            return new OcrResponse(OcrCode.fromValue((int)Double.parseDouble(rawJsonObj.get("code").toString())), rawJsonObj.get("data").toString());
        }

        return gson.fromJson(resp, OcrResponse.class);
    }

    @Override
    public void close() {
        if (p.isAlive()) {
            p.destroy();
        }
    }

}
