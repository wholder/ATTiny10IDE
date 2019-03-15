import java.io.*;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import java.util.zip.CRC32;

class Utility {
  private static final String   StartMarker = "//:Begin Embedded Markdown Data (do not edit)";
  private static final String   EndMarker = "\n//:End Embedded Markdown Data";
  private static char[]    hex = {'0', '1', '2', '3', '4', '5', '6', '7',
                                  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

  static String[] parse (String line) {
    List<String> out = new ArrayList<>();
    if (line.length() > 0) {
      line = condenseWhitespace(line);
      String[] tmp = line.split(" ");
      if (tmp.length > 0) {
        out.add(tmp[0]);
        StringTokenizer tok = new StringTokenizer(line.substring(tmp[0].length()).trim(), ",");
        while (tok.hasMoreTokens()) {
          out.add(tok.nextToken().trim());
        }
      }
    }
    return out.toArray(new String[0]);
  }

  /**
   * Reformat String to reduce all whitespace to a single space
   * @param text Input text
   * @return Reformatted output
   */
  static String condenseWhitespace (String text) {
    StringTokenizer tok = new StringTokenizer(text);
    StringBuilder buf = new StringBuilder();
    while (tok.hasMoreTokens()) {
      String line = tok.nextToken();
      buf.append(line);
      buf.append(' ');
    }
    return buf.toString().trim();
  }

  static void saveFile (File file, String text) {
    try {
      FileOutputStream out = new FileOutputStream(file);
      out.write(text.getBytes(StandardCharsets.UTF_8));
      out.close();
    } catch (IOException ex) {
      ex.printStackTrace();
    }
  }

  static void saveFile (String file, String text) throws Exception {
    FileOutputStream fOut = new FileOutputStream(file);
    fOut.write(text.getBytes(StandardCharsets.UTF_8));
    fOut.close();
  }

  static String getFile (File file) throws IOException {
    FileInputStream fis = new FileInputStream(file);
    byte[] data = new byte[fis.available()];
    fis.read(data);
    fis.close();
    return new String(data, StandardCharsets.UTF_8);
  }

  static String getFile (String file) throws IOException {
    InputStream fis;
    if (file.startsWith("res:")) {
      fis = Utility.class.getClassLoader().getResourceAsStream(file.substring(4));
    } else {
      fis = new FileInputStream(file);
    }
    if (fis != null) {
      byte[] data = new byte[fis.available()];
      fis.read(data);
      fis.close();
      return new String(data, StandardCharsets.UTF_8);
    }
    throw new IllegalStateException("getFile() " + file + " not found");
  }

  /**
   * Recursively remove files and directories from directory "dir"
   * @param dir starting directory
   */
  static void removeFiles (File dir) {
    final File[] files = dir.listFiles();
    if (files != null) {
      for (File file : files) {
        if (file.isDirectory()) {
          removeFiles(file);
          file.delete();
        } else {
          file.delete();
        }
      }
    }
  }

  static Properties getResourceMap (String file) throws IOException {
    InputStream fis = Utility.class.getClassLoader().getResourceAsStream(file);
    Properties prop = new Properties();
    prop.load(fis);
    fis.close();
    return prop;
  }

  static String replaceTags (String src, Map tags) {
    Pattern pat = Pattern.compile("(\\*\\[(.*?)]\\*)");
    Matcher mat = pat.matcher(src);
    StringBuffer buf = new StringBuffer();
    while (mat.find()) {
      String tag = mat.group(2);
      String rep = (String) tags.get(tag);
      if (rep == null) {
        throw new IllegalStateException("Utility.replaceTags() Tag '" + tag + "' not defined");
      }
      try {
        mat.appendReplacement(buf, Matcher.quoteReplacement(rep));
      } catch (Exception ex) {
        throw (new IllegalStateException("tag = '" + tag + "'. rep = '" + rep + "'"));
      }
    }
    mat.appendTail(buf);
    return buf.toString();
  }

  static String runCmd (Process proc) {
    return Stream.of(proc.getErrorStream(), proc.getInputStream()).parallel().map((InputStream isForOutput) -> {
      StringBuilder output = new StringBuilder();
      try (
        BufferedReader br = new BufferedReader(new InputStreamReader(isForOutput))) {
        String line;
        while ((line = br.readLine()) != null) {
          output.append(line);
          output.append("\n");
        }
      } catch (IOException e) {
        throw new RuntimeException(e);
      }
      return output;
    }).collect(Collectors.joining());
  }

  static void runCmd (Process proc, ATTinyC.MyTextPane pane) {
    Stream.of(proc.getErrorStream(), proc.getInputStream()).parallel().map((InputStream isForOutput) -> {
      StringBuilder output = new StringBuilder();
      try (
          BufferedReader br = new BufferedReader(new InputStreamReader(isForOutput))) {
        String line;
        while ((line = br.readLine()) != null) {
          pane.append(line);
          pane.append("\n");
        }
      } catch (IOException e) {
        throw new RuntimeException(e);
      }
      return output;
    }).collect(Collectors.joining());
  }

  private static void copyResourceToDir (String fName, String tmpDir) throws IOException {
    InputStream fis = Utility.class.getClassLoader().getResourceAsStream(fName);
    if (fis != null) {
      byte[] data = new byte[fis.available()];
      fis.read(data);
      fis.close();
      File file = new File(tmpDir + (new File(fName)).getName());
      FileOutputStream fOut = new FileOutputStream(file);
      fOut.write(data);
      fOut.close();
    } else {
      throw new IllegalStateException("copyResourceToDir('" + fName + "', '" + tmpDir + "') " + " unable to copy");
    }
  }

  static void copyResourcesToDir (String base, String tmpDir) throws URISyntaxException, IOException {
    java.nio.file.FileSystem fileSystem = null;
    if (base != null) {
      try {
        File path = new File(tmpDir);
        if (!path.exists()) {
          path.mkdirs();
        }
        URL url = Utility.class.getResource(base);
        if (url != null) {
          URI uri = url.toURI();
          Path myPath;
          if (uri.getScheme().equals("jar")) {
            fileSystem = FileSystems.newFileSystem(uri, Collections.emptyMap());
            myPath = fileSystem.getPath("/" + base);
          } else {
            myPath = Paths.get(uri);
          }
          Stream<Path> walk = Files.walk(myPath, 1);
          for (Iterator<Path> it = walk.iterator(); it.hasNext(); ) {
            Path item = it.next();
            String fName = item.getFileName().toString();
            if (!fName.equals(base)) {
              copyResourceToDir(base + "/" + fName, tmpDir);
            }
          }
        }
      } finally {
        if (fileSystem != null) {
          fileSystem.close();
        }
      }
    }
  }

  /**
   * Scans input code for a comment block containing encoded markdown text and, if present, extracts and
   * decodes it along with the source code (minus the comment block)
   * @param src input source code with optional encoded and embedded comment block
   * @return String[] array of length 1 of no embedded markdown, else String[] of length 2 where first
   * element in the array if the source code (mius the block comment) and the 2nd element is the extracted
   * and decoded markdown text.
   */
  static String[] decodeMarkdown (String src) {
    try {
      List<String> list = new ArrayList<>();
      int idx1 = src.indexOf(StartMarker);
      int idx2 = src.indexOf(EndMarker);
      if (idx1 >= 0 && idx2 > idx1) {
        list.add(src.substring(0, idx1) + src.substring(idx2 + EndMarker.length()));
        String tmp = src.substring(idx1 + StartMarker.length(), idx2);
        StringTokenizer tok = new StringTokenizer(tmp, "\n");
        StringBuilder buf = new StringBuilder();
        while (tok.hasMoreElements()) {
          String line = (String) tok.nextElement();
          if (line.startsWith("//:") && line.length() == 128 + 3) {
            buf.append(line.substring(3));
          }
        }
        tmp = new String(Base64.getDecoder().decode(buf.toString()), StandardCharsets.UTF_8);
        tmp = URLDecoder.decode(tmp, "utf8");
        list.add(tmp);
      } else {
        list.add(src);
      }
      return list.toArray(new String[0]);
    } catch (UnsupportedEncodingException ex) {
      ex.printStackTrace();
      return new String[] {src};
    }
  }

  /**
   * Adds path + filename to CRC32 crc
   */
  private static void crcFilename (Path path, CRC32 crc) {
    String file = path.getFileName().toString();
    if (!".DS_Store".equals(file)) {
      String val = path.toString();
      crc.update(val.getBytes(StandardCharsets.UTF_8));
    }
  }

  /**
   * Computes crc for tree of filenames and paths
   * @param base base of file tree
   */
  static long crcTree (String base) {
    CRC32 crc = new CRC32();
    try {
      Files.walk(Paths.get(base))
          .filter(path -> !Files.isDirectory(path))
          .forEach((path) -> crcFilename(path, crc));
    } catch (Exception ex) {
      return 0;
    }
    return crc.getValue();
  }

  static long crcZipfile (String srcZip) {
    CRC32 crc = new CRC32();
    try {
      InputStream in = Utility.class.getClassLoader().getResourceAsStream(srcZip);
      int size = in.available();
      byte[] data = new byte[size];
      in.read(data);
      crc.update(data);
    } catch (Exception ex) {
      return 0;
    }
    return crc.getValue();
  }

  static int fromHex (char cc) {
    cc = Character.toUpperCase(cc);
    return cc >= 'A' ? cc - 'A' + 10 : cc - '0';
  }

  static char hexChar (byte val) {
    return hex[val & 0x0F];
  }

  static boolean bit (int val, int bit) {
    return (val & (1 << bit)) != 0;
  }

  /**
   * JSON-like parser for non nested JSON Map
   * @param json JSON-like string, such as {fred=\"test\", alpha=\"123\"}"
   * @return Map of key/value pairs
   */
  static Map<String,String> parseJSON (String json) {
    Map<String,String> out = new HashMap<>();
    int start = json.indexOf("{");
    int end = json.indexOf("}", start + 1);
    if (start >= 0 && end >= start) {
      String tmp = json.substring(start + 1, end);
      String[] parts = tmp.split(",");
      for (String item : parts) {
        String[] pair = item.split("=");
        if (pair.length == 2) {
          String val = pair[1].replace('\"', ' ').trim();
          out.put(pair[0].trim(), val);
        }
      }
    }
    return out;
  }
}
