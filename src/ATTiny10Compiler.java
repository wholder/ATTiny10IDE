import java.io.*;

import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;

  /*
   * GNU Toolchain Controller for Compiling and Assembling code for ATTin10 Series Chips
   * Author: Wayne Holder, 2017
   * License: MIT (https://opensource.org/licenses/MIT)
   */
class ATTiny10Compiler {
  private static String     fileSep =  System.getProperty("file.separator");
  private static char[]     hex = {'0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  private static String[][] asm = {
    {"COMP1", "avr-as -mmcu=*[CHIP]* -I *[IDIR]* -o *[TDIR]*code.o *[TDIR]*code.S "},
    {"COMP2", "avr-ld -mavrtiny -o *[TDIR]*code.elf *[TDIR]*code.o"},
    {"TOHEX", "avr-objcopy -j .text -j .data -O ihex *[TDIR]*code.elf *[TDIR]*code.hex"},
    {"LST",   "avr-objdump -d -S -t *[TDIR]*code.elf"},
    {"SIZE",  "avr-size --format=avr --mcu=*[CHIP]* *[TDIR]*code.elf"},
  };
  private static String[][] comp = {
    {"COMP1", "avr-g++ -ggdb -std=gnu++11 -Wall -Wno-unknown-pragmas -Os -DF_CPU=*[CLOCK]* -mmcu=*[CHIP]* -c *[TDIR]*code.c -o *[TDIR]*code.o"},
    {"COMP2", "avr-g++ -std=gnu++11 -Wall -Os -DF_CPU=*[CLOCK]* -mmcu=*[CHIP]* -o *[TDIR]*code.elf *[TDIR]*code.o"},
    {"TOHEX", "avr-objcopy -j .text -j .data -O ihex *[TDIR]*code.elf *[TDIR]*code.hex"},
    {"LST",   "avr-objdump -d -S -t *[TDIR]*code.elf"},
    {"SIZE",  "avr-size --format=avr --mcu=*[CHIP]* *[TDIR]*code.elf"},
  };
  private static Map<String,Integer>  fuses = new HashMap<>();

  static {
    // Define fuse bits
    fuses.put("ckout", 4);    // System Clock Output
    fuses.put("wdton", 2);    // Watchdog Timer Always On
    fuses.put("rstdisbl", 1); // External Reset Disable
  }

  public Map<String,String> compile (String src, String tmpExe, String tmpDir, boolean doAsm) throws Exception {
    // Remove any prior tmp files
    final File[] files = (new File(tmpDir)).listFiles();
    if (files != null) {
      for (File file : files) {
        file.delete();
      }
    }
    byte fuseBits = 0x0F;
    String clock = null, chip = null;
    // Process #pragma directives
    StringTokenizer tok = new StringTokenizer(src, "\n\r");
    while (tok.hasMoreTokens()) {
      String line = tok.nextToken().trim();
      if (line.startsWith("#pragma")) {
        line = line.substring(7).trim();
        String[] parts = parse(line);
        if (parts.length > 1) {
          if ("fuses".equals(parts[0])) {
            byte tmp = 0;
            for (int ii = 1; ii < parts.length; ii++) {
              if (fuses.containsKey(parts[ii])) {
                tmp |= (byte) fuses.get(parts[ii]).intValue();
              } else {
                System.out.println("#pragma unknown fuse: " + parts[ii]);
                Map<String,String> out = new HashMap<>();
                out.put("ERR", "#pragma unknown fuse: " + parts[ii]);
                return out;
              }
            }
            fuseBits = (byte) ~tmp;
          } else if ("clock".equals(parts[0])) {
            clock = parts[1];
          } else if ("chip".equals(parts[0])) {
            chip = parts[1];
            chip = chip != null ? chip : "attiny10";
         }
        }
      }
    }
    Map<String,String> tags = new HashMap<>();
    String curDir = new File("").getAbsolutePath() +  fileSep;
    tags.put("CDIR", curDir);
    tags.put("TDIR", tmpDir);
    tags.put("IDIR", tmpExe + "avr" + fileSep + "include" + fileSep);
    tags.put("CHIP", chip);
    tags.put("CLOCK", clock != null ? clock : "8000000");
    // Copy contents of "source" pane to temp file with appropriate extension for code type
    try {
      FileOutputStream fOut = new FileOutputStream(tmpDir + (doAsm ? "code.S" : "code.c"));
      fOut.write(src.getBytes("UTF8"));
      fOut.close();
    } catch (IOException ex) {
      tags.put("ERR", ex.toString());
    }
    // Compile Sequence
    Map<String,String> out = new HashMap<>();
    out.put("INFO", "chip: " + tags.get("CHIP") + ", clock: " + tags.get("CLOCK") + ", fuses: " + hexChar(fuseBits));
    for (String[] seq : (doAsm ? asm : comp)) {
      String cmd = tmpExe + "bin" + fileSep + seq[1];
      try {
      cmd = replaceTags(cmd, tags);
      System.out.println("Run: " + cmd);
      Process proc = Runtime.getRuntime().exec(cmd);
      String ret = Stream.of(proc.getErrorStream(), proc.getInputStream()).parallel().map((InputStream isForOutput) -> {
        StringBuilder output = new StringBuilder();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(isForOutput))) {
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
      int retVal = proc.waitFor();
      if (retVal != 0) {
        tags.put("ERR", ret);
        return tags;
      }
      out.put(seq[0], ret);
     } catch (IllegalStateException ex) {
        ex.printStackTrace();
        tags.put("ERR", ex.getMessage());
        return tags;
      }
    }
    String hex = new String(getFile(tmpDir + "code.hex"));
    String buf = ":020000020000FC\n" +  // Set origin at 0
        "*" + hexChar(fuseBits) + "\n" + hex;
    out.put("HEX", buf);
    out.put("CHIP", chip);
    return out;
  }

  private static String replaceTags (String src, Map tags) {
    Pattern pat = Pattern.compile("(\\*\\[(.*?)]\\*)");
    Matcher mat = pat.matcher(src);
    StringBuffer buf = new StringBuffer();
    while (mat.find()) {
      String tag = mat.group(2);
      String rep = (String) tags.get(tag);
      try {
        mat.appendReplacement(buf, rep != null ? Matcher.quoteReplacement(rep) : "");
      } catch (Exception ex) {
        throw (new IllegalStateException("tag = '" + tag + "'. rep = '" + rep + "'"));
      }
    }
    mat.appendTail(buf);
    return buf.toString();
  }

  private static byte[] getFile (String file) throws IOException {
    InputStream fis;
    if (file.startsWith("res:")) {
      fis = Object.class.getResourceAsStream("/" + file.substring(4));
    } else {
      fis = new FileInputStream(file);
    }
    byte[] data = new byte[fis.available()];
    fis.read(data);
    fis.close();
    return data;
  }

  private static String[] parse (String line) {
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
  private static String condenseWhitespace (String text) {
    StringTokenizer tok = new StringTokenizer(text);
    StringBuilder buf = new StringBuilder();
    while (tok.hasMoreTokens()) {
      String line = tok.nextToken();
      buf.append(line);
      buf.append(' ');
    }
    return buf.toString().trim();
  }

  static char hexChar (byte val) {
    return hex[val & 0x0F];
  }
}
