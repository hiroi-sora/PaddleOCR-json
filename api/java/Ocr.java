package org.example;

import com.google.gson.Gson;
import com.google.gson.annotations.SerializedName;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.io.FilterWriter;
import java.io.IOException;
import java.io.Writer;

// from: https://github.com/soot-oss/soot/blob/3966f565db6dc2882c3538ffc39e44f4c14b5bcf/src/main/java/soot/util/EscapedWriter.java
/*-
 * #%L
 * Soot - a J*va Optimization Framework
 * %%
 * Copyright (C) 1997 - 1999 Raja Vallee-Rai
 * %%
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Lesser Public License for more details.
 *
 * You should have received a copy of the GNU General Lesser Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * #L%
 */
/**
 * A FilterWriter which catches to-be-escaped characters (<code>\\unnnn</code>) in the input and substitutes their escaped
 * representation. Used for Soot output.
 */
class EscapedWriter extends FilterWriter {
    /** Convenience field containing the system's line separator. */
    public final String lineSeparator = System.getProperty("line.separator");
    private final int cr = lineSeparator.charAt(0);
    private final int lf = (lineSeparator.length() == 2) ? lineSeparator.charAt(1) : -1;

    /** Constructs an EscapedWriter around the given Writer. */
    public EscapedWriter(Writer fos) {
        super(fos);
    }

    private final StringBuffer mini = new StringBuffer();

    /** Print a single character (unsupported). */
    public void print(int ch) throws IOException {
        write(ch);
        throw new RuntimeException();
    }

    /** Write a segment of the given String. */
    public void write(String s, int off, int len) throws IOException {
        for (int i = off; i < off + len; i++) {
            write(s.charAt(i));
        }
    }

    /** Write a single character. */
    public void write(int ch) throws IOException {
        if (ch >= 32 && ch <= 126 || ch == cr || ch == lf || ch == ' ') {
            super.write(ch);
            return;
        }

        mini.setLength(0);
        mini.append(Integer.toHexString(ch));

        while (mini.length() < 4) {
            mini.insert(0, "0");
        }

        mini.insert(0, "\\u");
        for (int i = 0; i < mini.length(); i++) {
            super.write(mini.charAt(i));
        }
    }
}

class OcrCode {
    public static final int OK = 100;
    public static final int NO_TEXT = 101;
}

class OcrEntry {
    String text;
    int[][] box;
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
    int code;
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

    public OcrResponse(int code, String msg) {
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

        if (!commands.contains("use_debug")) {
            commands += ' ' + "--use_debug=0";
        }

        if (!StandardCharsets.US_ASCII.newEncoder().canEncode(commands)) {
            throw new IllegalArgumentException("参数不能含有非 ASCII 字符");
        }

        System.out.println("当前参数：" + commands);

        File workingDir = exePath.getParentFile();
        ProcessBuilder pb = new ProcessBuilder(exePath.toString(), commands);
        pb.directory(workingDir);
        pb.redirectErrorStream(true);
        p = pb.start();

        InputStream stdout = p.getInputStream();
        OutputStream stdin = p.getOutputStream();
        reader = new BufferedReader(new InputStreamReader(stdout, StandardCharsets.UTF_8));
        writer = new BufferedWriter(new OutputStreamWriter(stdin, StandardCharsets.UTF_8));

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
        return this.runOcrOnPath(imgFile.toString());
    }

    public OcrResponse runOcrOnClipboard() throws IOException {
        return this.runOcrOnPath("clipboard");
    }

    private OcrResponse runOcrOnPath(String path) throws IOException {
        if (!p.isAlive()) {
            throw new RuntimeException("OCR进程已经退出");
        }
        Map<String, String> reqJson = new HashMap<>();
        reqJson.put("image_dir", path);
        StringWriter sw = new StringWriter();
        EscapedWriter ew = new EscapedWriter(sw);
        gson.toJson(reqJson, ew);
        writer.write(sw.getBuffer().toString());
        writer.write("\r\n");
        writer.flush();
        String resp = reader.readLine();
        System.out.println(resp);

        Map rawJsonObj = gson.fromJson(resp, Map.class);
        if (rawJsonObj.get("data") instanceof String) {
            return new OcrResponse((int)Double.parseDouble(rawJsonObj.get("code").toString()), rawJsonObj.get("data").toString());
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
