import sun.nio.cs.UTF_32;

import java.io.*;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
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
  private static Utility   util = new Utility();
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

  static String pad (String val, int len) {
    StringBuilder buf = new StringBuilder(val);
    while (buf.length() < len) {
      buf.append(" ");
    }
    return buf.toString();
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
      fis = util.getClass().getResourceAsStream(file.substring(4));
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

  static Properties getResourceMap (String file) throws IOException {
    InputStream fis = util.getClass().getResourceAsStream("/" + file);
    Properties prop = new Properties();
    prop.load(fis);
    fis.close();
    return prop;
  }

  static Map<String,String> toTreeMap (Properties props) {
    Map<String,String> map = new TreeMap<>();
    for (final String name: props.stringPropertyNames()) {
      map.put(name, props.getProperty(name));
    }
    return map;
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

  static void copyResourceToDir (String fName, String tmpDir) throws IOException {
    InputStream fis = util.getClass().getResourceAsStream(fName);
    if (fis != null) {
      byte[] data = new byte[fis.available()];
      fis.read(data);
      fis.close();
      File file = new File(tmpDir + (new File(fName)).getName());
      FileOutputStream fOut = new FileOutputStream(file);
      fOut.write(data);
      fOut.close();
    } else {
      throw new IllegalStateException("copyResourceToDir() " + fName + " unable to copy");
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
   * @param path
   * @param crc
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
      InputStream in = util.getClass().getResourceAsStream(srcZip);
      int size = in.available();
      byte[] data = new byte[size];
      in.read(data);
      crc.update(data);
    } catch (Exception ex) {
      return 0;
    }
    return crc.getValue();
  }

  /*
  public static void main (String[] args) {
    long start = System.currentTimeMillis();
    long checksum = crcZipfile("toolchains/MacToolchain.zip");
    long end = System.currentTimeMillis();
    long elapsed = end - start;
    int sum = 0;
  }
  */

  static int fromHex (char cc) {
    cc = Character.toUpperCase(cc);
    return cc >= 'A' ? cc - 'A' + 10 : cc - '0';
  }

  static char hexChar (byte val) {
    return hex[val & 0x0F];
  }

  static String choose (String choice1, String choice2) {
    return choice1 != null ? choice1 : choice2;
  }

  static Map<String,String> csvToMap (String csv) {
    Map<String,String> map = new HashMap<>();
    for (String item : csv.split(",")) {
      String[] tmp = item.split(":");
      map.put(tmp[0], tmp[1]);
    }
    return map;
  }

  static boolean bit (int val, int bit) {
    return (val & (1 << bit)) != 0;
  }
}
