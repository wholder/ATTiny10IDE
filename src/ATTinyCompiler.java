import cpp14.CPP14ProtoGen;

import javax.swing.*;
import java.io.*;

import java.util.*;
import java.util.prefs.Preferences;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/*
   * GNU Toolchain Controller for Compiling and Assembling code for ATTiny10 Series Chips
   * Author: Wayne Holder, 2017
   * License: MIT (https://opensource.org/licenses/MIT)
   *
   * References:
   *    https://www.mankier.com/1/arduino-ctags - used by Arduino IDE to generate function prototypes
   */

class ATTinyCompiler {
  private static final String fileSep = System.getProperty("file.separator");

  private static final String prePro =  "avr-g++ " +                  // https://linux.die.net/man/1/avr-g++
                                        "-w " +                       // Inhibit all warning messages
                                        "-x c++ " +                   // Assume c++ file
                                        "-E " +                       // Preprocess only
                                        "-MMD " +                     // Generate dependencies to Sketch.inc
                                        "-I *[TDIR]* " +              // Also search in temp directory for header files
                                        "-MF *[TDIR]*Sketch.inc " +   //   " "
                                        "-DF_CPU=*[CLOCK]* " +        // Create #define for F_CPU
                                        "-mmcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                        "-DARDUINO_ARCH_AVR " +       // #define ARDUINO_ARCH_AVR
                                        "*[TDIR]**[IFILE]* ";         // Source file is temp/IFILE.x

  private static final String compCpp = "avr-g++ " +                  // https://linux.die.net/man/1/avr-g++
                                        "-c " +                       // Compile but do not link
                                        "-g " +                       // Enable link-time optimization
                                        "-Os " +                      // Optimize for size
                                        "-w " +                       // Inhibit all warning messages
                                        "-std=gnu++11 " +             // Support GNU extensions to C++
                                        "-fpermissive " +             // Downgrade nonconformant code errors to warnings
                                        "-fno-exceptions " +          // Disable exception-handling code
                                        "-ffunction-sections " +      // Separate functions in output file
                                        "-fdata-sections " +          // Separate data in output file
                                        "-fno-threadsafe-statics " +  // No extra code for C++ ABI routines
                                        "-MMD " +                     // Mention only user header files
                                        "-flto " +                    // Run standard link optimizer (requires 5.4.0)
                                        "-DLTO_ENABLED " +
                                        "-mmcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                        "-DF_CPU=*[CLOCK]* " +        // Create #define for F_CPU
                                        "-DARDUINO_ARCH_AVR " +       // #define ARDUINO_ARCH_AVR
                                        "*[DEFINES]* " +              // Add in conditional #defines, if any
                                        "-I *[TDIR]* " +              // Also search in temp directory for header files
                                        "*[TDIR]**[IFILE]* " +        // Source file is temp/IFILE.x
                                        "-o *[TDIR]**[IFILE]*.o ";    // Output to file temp/IFILE.x.o

  private static final String compC = "avr-gcc " +                    // https://linux.die.net/man/1/avr-gcc
                                        "-c " +                       // Compile but do not link
                                        "-g " +                       // Enable link-time optimization
                                        "-Os " +                      // Optimize for size
                                        "-w " +                       // Inhibit all warning messages.
                                        "-std=gnu11 " +
                                        "-ffunction-sections " +      // Separate functions in output file
                                        "-fdata-sections " +          // Separate data in output file
                                        "-flto " +                    // Run standard link optimizer (requires 5.4.0)
                                        "-DLTO_ENABLED " +
                                        "-DF_CPU=*[CLOCK]* " +        // Create #define for F_CPU
                                        "-mmcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                        "-DARDUINO_ARCH_AVR " +       // #define ARDUINO_ARCH_AVR
                                        "*[DEFINES]* " +              // Add in conditional #defines, if any
                                        "-MMD " +                      // Mention only user header files
                                        "-fno-fat-lto-objects " +     //
                                        "-I *[TDIR]* " +              // Also search in temp directory for header files
                                        "*[TDIR]**[IFILE]* " +        // Source file is temp/IFILE.x
                                        "-o *[TDIR]**[IFILE]*.o ";    // Output to file temp/IFILE.x.o

  private static final String compAsm = "avr-gcc " +                  // https://linux.die.net/man/1/avr-gcc
                                        "-c " +                       // Compile but do not link
                                        "-g " +                       // Enable link-time optimization
                                        "-x assembler-with-cpp " +    //
                                        "-flto " +                    // Run standard link optimizer (requires 5.4.0)
                                        "-DLTO_ENABLED " +            //
                                        "-DF_CPU=*[CLOCK]* " +        // Create #define for F_CPU
                                        "-mmcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                        "-DARDUINO_ARCH_AVR " +       // #define ARDUINO_ARCH_AVR
                                        "*[DEFINES]* " +              // Add in conditional #defines, if any
                                        "-I *[TDIR]* " +              // Also search in temp directory for header files
                                        "*[TDIR]**[IFILE]* " +        // Source file is temp/IFILE.x
                                        "-o *[TDIR]**[IFILE]*.o ";    // Output to file temp/IFILE.x.o

  private static final String link = "avr-gcc " +                     // https://linux.die.net/man/1/avr-g++
                                        "-w " +                       // Inhibit all warning messages.
                                        "-Os " +                      // Optimize for size
                                        "-g " +                       // Enable link-time optimization
                                        "-flto " +                    // Run standard link optimizer (requires 5.4.0)
                                        "-DLTO_ENABLED " +            // ??
                                        "-fuse-linker-plugin " +      // Enable link-time optimization (requires 5.4.0)
                                        "-Wl,--gc-sections " +        // Eliminate unused code
                                        "-Wl,--print-gc-sections " +  // Print dead code removed
                                        "-DF_CPU=*[CLOCK]* " +        // Create #define for F_CPU
                                        "-mmcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                        "-o *[TDIR]**[OFILE]* " +     // Output to file temp/OFILE
                                        "*[LIST]*" +                  // List of files to link (each prefixed by temp/)
                                        "-L*[TDIR]* " +               // Also search in temp dir for -l option
                                        "-lm ";                       // Link Math library (??)

  private static final String list = "avr-objdump " +                 // https://linux.die.net/man/1/avr-objdump
                                        "-d " +                       // Disassemble code
                                        "*[INTLV]* " +                // Display source  code  intermixed  with  disassembly
                                        "-t " +                       // Print the symbol table entries
                                        "*[TDIR]*Sketch.elf";         // Input file

  private static final String tohex = "avr-objcopy " +                // https://linux.die.net/man/1/avr-objcopy
                                        "-O ihex " +                  // Output format is Intel HEX
                                        "-R .eeprom " +               // Remove .eeprom section (!!!)
                                        "*[TDIR]*Sketch.elf " +       // Input file
                                        "*[TDIR]*Sketch.hex";         // Output file

  private static final String size = "avr-size " +                    // https://linux.die.net/man/1/avr-size
                                        "-A " +                       //
                                        "*[TDIR]*Sketch.elf";         // Input file

  private static String[][] asm = {
      {"COMP1", "avr-as -mmcu=*[CHIP]* -I *[IDIR]* *[TDIR]*Sketch.S -o *[TDIR]*Sketch.o "},
      {"COMP2", "avr-ld -mavrtiny *[TDIR]*Sketch.o -o *[TDIR]*Sketch.elf "},
      {"TOHEX", tohex},
      {"LST", list},
      {"SIZE", size},
  };

  private static String[][] build = {
      {"TOHEX", tohex},
      {"LST", list},   // Note add "-l' for source path and line numbers (Warning: large lines!)
      {"SIZE", size},
  };

  private static Map<String, Integer> fuses = new HashMap<>();

  static {
    // Define fuse bits
    fuses.put("ckout", 4);    // System Clock Output
    fuses.put("wdton", 2);    // Watchdog Timer Always On
    fuses.put("rstdisbl", 1); // External Reset Disable
  }

  static Map<String, String> compile (String src, Map<String, String> tags, Preferences prefs, JFrame tinyIde) throws Exception {
    String tmpDir = tags.get("TDIR");
    String tmpExe = tags.get("TEXE");
    String srcName = tags.get("FNAME").toLowerCase();
    Utility.removeFiles(new File(tmpDir));
    boolean doAsm = srcName.endsWith(".s");
    boolean isCCode = srcName.endsWith(".c") || srcName.endsWith(".cpp") || srcName.endsWith(".ino");
    boolean preOnly = isCCode && "PREONLY".equals(tags.get("PREPROCESS"));
    boolean genProto = isCCode && "GENPROTOS".equals(tags.get("PREPROCESS"));
    byte fuseBits = 0x0F;
    String clock = null;
    String chip = "attiny10";
    StringBuilder defines = new StringBuilder();
    Map<String, String> out = new HashMap<>();
    List<String> warnings = new ArrayList<>();
    Map<String, Integer[]> exports = new LinkedHashMap<>();
    // Process #pragma and #include directives
    int lineNum = 0;
    int LastIncludeLine = 1;
    for (String line : src.split("\n")) {
      lineNum++;
      int idx = line.indexOf("//");
      if (idx >= 0) {
        line = line.substring(0, idx).trim();
      }
      if (line.startsWith("#pragma")) {
        line = line.substring(7).trim();
        String[] parts = Utility.parse(line);
        if (parts.length > 1) {
          tags.put("PRAGMA." + parts[0].toUpperCase(), parts[1]);
          switch (parts[0]) {
            case "fuses":                                         // Attiny4,5,9,10 fuse bits
              byte tmp = 0;
              for (int ii = 1; ii < parts.length; ii++) {
                if (fuses.containsKey(parts[ii])) {
                  tmp |= (byte) fuses.get(parts[ii]).intValue();
                } else {
                  System.out.println("#pragma unknown fuse: " + parts[ii]);
                  out.put("ERR", "#pragma unknown fuse: " + parts[ii]);
                  return out;
                }
              }
              fuseBits = (byte) ~tmp;
              out.put("FUSES", "0x" + Integer.toHexString(fuseBits));
              break;
            case "clock":                                         // Sets F_CPU #define
              clock = parts[1];
              break;
            case "chip":                                          // Sets -mmcu compile option
              chip = parts[1];
              break;
            case "define":                                        // Sets -D compile option to parts[1]
              defines.append("-D").append(parts[1]).append(" ");
              break;
            case "hfuse":
              out.put("HFUSE", parts[1]);                         // Sets value of *[HFUSE]* tag in "out" Map
              break;
            case "lfuse":
              out.put("LFUSE", parts[1]);                         // Sets value of *[LFUSE]* tag in "out" Map
              break;
            case "efuse":
              out.put("EFUSE", parts[1]);                         // Sets value of *[EFUSE]* tag in "out" Map
              break;
            case "xparm":                                         // Defines exported parameter
              exports.put(parts[1], new Integer[0]);
              break;
            default:
              warnings.add("Unknown pragma: " + line + " (ignored)");
              break;
          }
        } else {
          warnings.add("Invalid pragma: " + line + " (ignored)");
        }
      } else if (line.startsWith("#include")) {
        LastIncludeLine = Math.max(LastIncludeLine, lineNum);
      }
    }
    tags.put("CHIP", chip);
    tags.put("INTLV", prefs.getBoolean("interleave", true) ? "-S" : "");
    tags.put("CLOCK", clock != null ? clock : "8000000");
    tags.put("DEFINES", defines.toString());
    // Build list of files we need to compile and link
    ATTinyC.ChipInfo chipInfo = ATTinyC.progProtocol.get(chip.toLowerCase());
    if (chipInfo == null) {
      throw new IllegalStateException("Unknown chip type: " + chip);
    }
    if ("TPI".equals(chipInfo.prog)) {
      out.put("INFO", "chip: " + chip + ", clock: " + tags.get("CLOCK") + ", fuses: " + Utility.hexChar(fuseBits));
    } else {
      out.put("INFO", "chip: " + chip + ", clock: " + tags.get("CLOCK") + ", lfuse: " + out.get("LFUSE") +
          ", hfuse: " + out.get("HFUSE") + ", efuse: " + out.get("EFUSE"));
    }
    ATTinyC.ProgressBar progress = null;
    try {
      // Copy contents of "source" pane to Sketch file with appropriate extension for code type
      String mainFile = doAsm ? "Sketch.S" : "Sketch.cpp";
      Utility.saveFile(tmpDir + mainFile, src);
      // Run Compile and Link Sequences
      if (doAsm) {
        // Assemble AVR code using GNU assembler
        for (String[] seq : asm) {
          String cmd = Utility.replaceTags(tmpExe + "bin" + fileSep + seq[1], tags);
          System.out.println("Run: " + cmd);
          Process proc = Runtime.getRuntime().exec(cmd);
          String ret = Utility.runCmd(proc);
          int retVal = proc.waitFor();
          if (retVal != 0) {
            tags.put("ERR", ret);
            return tags;
          }
          out.put(seq[0], ret);
        }
      } else {
        List<String> compFiles = new ArrayList<>();
        compFiles.add(mainFile);
        // Copy "core" and core "variant" files into tmpDir so compiler can reference them
        Utility.copyResourcesToDir(chipInfo.core, tmpDir);
        Utility.copyResourcesToDir(chipInfo.variant, tmpDir);
        File[] coreFiles = (new File(tmpDir)).listFiles();
        // Copy "lib" files into tmpDir so compiler can reference them
        Utility.copyResourcesToDir(chipInfo.libs, tmpDir);
        if (preOnly || genProto) {
          try {
            // Preprocess .cpp source code using GNU c++ compiler
            tags.put("IFILE", "Sketch.cpp");
            String cmd = Utility.replaceTags(tmpExe + "bin" + fileSep + prePro, tags);
            System.out.println("Run: " + cmd);
            Process proc = Runtime.getRuntime().exec(cmd);
            String ret = Utility.runCmd(proc);
            int retVal = proc.waitFor();
            if (retVal != 0) {
              tags.put("ERR", ret);
              return tags;
            }
            // Scan preprocess output for line markers and extract section for Sketch.cpp
            StringTokenizer lines = new StringTokenizer(ret, "\n");
            StringBuilder buf = new StringBuilder();
            boolean inSketch = false;
            boolean lineMarkerFound = false;
            Pattern lMatch = Pattern.compile("#\\s\\d+\\s\"(.*?)\"");
            String pathPat = tmpDir + "Sketch.cpp";
            String osName = System.getProperty("os.name").toLowerCase();
            if (osName.contains("win")) {
              pathPat = pathPat.replace("\\", "\\\\");
            }
            while (lines.hasMoreElements()) {
              String line = lines.nextToken();
              Matcher mat = lMatch.matcher(line);
              if (mat.find()) {
                String seq = mat.group(1);
                inSketch = seq.equals(pathPat);
                if (!lineMarkerFound) {
                  buf.append(line).append("\n");
                  lineMarkerFound = true;
                }
              } else if (inSketch) {
                buf.append(line);
                buf.append("\n");
              }
            }
            // Generate prototypes
            String protos = CPP14ProtoGen.getPrototypes(buf.toString());
            if (genProto) {
              // Copy protos and source into Sketch.cpp and continue build
              lineNum = 0;
              buf = new StringBuilder();
              for (String line : src.split("\n")) {
                lineNum++;
                buf.append(line).append("\n");
                if (lineNum == LastIncludeLine) {
                  buf.append(protos).append("#line ").append(lineNum + 1).append("\n");
                }
              }
              Utility.saveFile(tmpDir + mainFile, buf.toString());
            } else {
              // Return just the preprocessed source
              out.put("PRE", ret);
              return out;
            }
          } catch (Exception ex) {
            ex.printStackTrace();
            tags.put("ERR", ex.getMessage());
            return tags;
          }
        }
        StringBuilder linkList = new StringBuilder();
        Map<String,String> codeFiles = new HashMap<>();
        File[] files = (new File(tmpDir)).listFiles();
        if (files != null) {
          for (File file : files) {
            String fName = file.getName();
            String[] parts = fName.toLowerCase().split("\\.");
            if (parts.length > 1 && ("cpp".equals(parts[1]) || "c".equals(parts[1]))) {
              codeFiles.put(parts[0], fName);
            }
          }
        }
        // Compile source code and add in included code files as they are discovered
        progress = new ATTinyC.ProgressBar(tinyIde, "Compiling and Building");
        progress.setMaximum(compFiles.size());
        int progCount = 0;
        progress.setValue(progCount);
        for (int ii = 0; ii < compFiles.size(); ii++) {
          String compFile = compFiles.get(ii);
          linkList.append(tmpDir).append(compFile).append(".o ");
          String cmd = getCompileCommand(compFile, tags);
          System.out.println("Run: " + cmd);
          Process proc = Runtime.getRuntime().exec(cmd);
          String ret = Utility.runCmd(proc);
          int retVal = proc.waitFor();
          if (retVal != 0) {
            String msg = "While Compiling\n" + ret;
            System.out.println(msg);
            tags.put("ERR", msg);
            return tags;
          }

          // Scan .d file for include files that need to also be compiled
          String buf = Utility.getFile(tmpDir + compFile + ".d");
          StringTokenizer tok = new StringTokenizer(buf, "\n");
          while (tok.hasMoreElements()) {
            String line = tok.nextToken().trim();
            line = line.substring(tmpDir.length());
            if (!line.contains(mainFile)) {
              if (line.endsWith("\\")) {
                line = line.substring(0, line.length() - 1).trim();
              }
              String[] parts = line.split("\\.");
              if (parts.length > 1 && "h".equals(parts[1])) {
                String codeFile = codeFiles.get(parts[0].toLowerCase());
                if (codeFile != null) {
                  // Add in code file matching #included header
                  if (!compFiles.contains(codeFile)) {
                    compFiles.add(codeFile);
                    progress.setMaximum(compFiles.size());
                  }
                } else if ("arduino".equals(parts[0].toLowerCase())) {
                  // Add all core files into the list of source files to compile (can we improve this?)
                  if (coreFiles != null) {
                    for (File file : coreFiles) {
                      String fName = file.getName();
                      String[] cParts = fName.toLowerCase().split("\\.");
                      if (cParts.length == 2 && ("cpp".equals(cParts[1]) || "c".equals(cParts[1]))) {
                        if (!compFiles.contains(fName)) {
                          compFiles.add(fName);
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          progress.setValue(++progCount);
        }
        // Link all object files
        tags.put("LIST", linkList.toString());
        tags.put("OFILE", "Sketch.elf");
        String cmd = Utility.replaceTags(tmpExe + "bin" + fileSep + link, tags);
        System.out.println("Run: " + cmd);
        Process proc = Runtime.getRuntime().exec(cmd);
        String ret = Utility.runCmd(proc);
        int retVal = proc.waitFor();
        if (retVal != 0) {
          String msg = "While Linking\n" + ret;
          System.out.println(msg);
          tags.put("ERR", msg);
          return tags;
        }
        // Generate Arduino-like sketch hex output, listing and code/data size info
        for (String[] seq : build) {
          cmd = Utility.replaceTags(tmpExe + "bin" + fileSep + seq[1], tags);
          System.out.println("Run: " + cmd);
          proc = Runtime.getRuntime().exec(cmd);
          ret = Utility.runCmd(proc);
          retVal = proc.waitFor();
          if (retVal != 0) {
            String msg = "While Building\n" + ret;
            System.out.println(msg);
            tags.put("ERR", msg);
            return tags;
          }
          out.put(seq[0], ret);
        }
      }
    } catch (Exception ex) {
      ex.printStackTrace();
      tags.put("ERR", ex.getMessage());
      return tags;
    } finally {
      if (progress != null) {
        progress.close();
      }
    }
    // Copy pragma values into "out" Map
    for (String key : tags.keySet()) {
      String val = tags.get(key);
      if (key.startsWith("PRAGMA.")) {
        out.put(key, val);
      }
    }
    String buf = Utility.getFile(tmpDir + "Sketch.hex");
    if ("TPI".equals(chipInfo.prog)) {
      // If attiny10 series, prefix with fuse settings
      buf = ":020000020000FC\n" +  // Set origin at 0
          "*" + Utility.hexChar(fuseBits) + "\n" + buf;
    }
    out.put("HEX", buf);
    out.put("CHIP", chip);
    // Check if any variables were exported
    if (exports.size() > 0) {
      try {
        String listing = out.get("LST");
        int idx = listing.indexOf("SYMBOL TABLE:\n");
        int loadStart = 0, dataStart = 0;
        if (idx >= 0) {
          int end = listing.indexOf("\n\n", idx);
          if (end >= 0 && end > idx) {
            String symbols = listing.substring(idx + 14, end);
            StringTokenizer tok = new StringTokenizer(symbols, "\n");
            while (tok.hasMoreElements()) {
              String line = Utility.condenseWhitespace(tok.nextToken());
              String[] parts = line.split("\\s");
              if (parts.length == 6) {
                if ("O".equals(parts[2]) && exports.containsKey(parts[5])) {
                  String tmp = parts[0];
                  int add = Integer.parseInt(tmp.substring(tmp.length() - 4), 16);
                  int size = Integer.parseInt(parts[4]);
                  exports.put(parts[5], new Integer[]{add, size});
                }
              } else if (parts.length == 5) {
                int add = Integer.parseInt(parts[0].toUpperCase().substring(parts[0].length() - 4), 16);
                switch (parts[4]) {
                  case "__data_load_start":
                    loadStart = add;
                    break;
                  case "__data_start":
                    dataStart = add;
                    break;
                }
              }
            }
          }
        }
        StringBuilder exVars = new StringBuilder();
        for (String name : exports.keySet()) {
          Integer[] parts = exports.get(name);
          if (parts != null && parts.length == 2) {
            int add = parts[0] - dataStart + loadStart;
            int size = parts[1];
            exVars.append(name).append(":").append(Integer.toHexString(add)).append(":").append(size).append("\n");
          } else {
            warnings.add("Data for #pragma xparm: " + name + " not found in .data section - " +
                "declare with __attribute__ ((section (\".data\")))");
          }
        }
        out.put("XPARMS", exVars.toString());
      } catch (Exception ex) {
        ex.printStackTrace();
      }
    }
    // Check if any warnings were generated
    if (warnings.size() > 0) {
      StringBuilder tmp = new StringBuilder("Warnings:\n");
      for (String warn : warnings) {
        tmp.append("  ").append(warn).append("\n");
      }
      out.put("WARN", tmp.toString());
    }
    return out;
  }

  private static String getCompileCommand (String compFile, Map<String, String> tags) {
    String tmpExe = tags.get("TEXE");
    String suffix = compFile.substring(compFile.indexOf("."));
    tags.put("IFILE", compFile);
    String cmd;
    switch (suffix.toLowerCase()) {
      case ".c":
        cmd = Utility.replaceTags(tmpExe + "bin" + fileSep + compC, tags);
        break;
      case ".cpp":
        cmd = Utility.replaceTags(tmpExe + "bin" + fileSep + compCpp, tags);
        break;
      case ".s":
        cmd = Utility.replaceTags(tmpExe + "bin" + fileSep + compAsm, tags);
        break;
      default:
        throw new IllegalStateException("Unknown file type: " + suffix);
    }
    return cmd;
  }
}
