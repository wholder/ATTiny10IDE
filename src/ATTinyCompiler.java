import cpp14.CPP14ProtoGen;

import javax.swing.*;
import java.io.*;

import java.util.*;
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
  private static final Pattern  libMatch = Pattern.compile("#include\\s+<([a-zA-Z.]+)>");
  private static final String   fileSep =  System.getProperty("file.separator");

  private static final String prePro = "avr-g++ " +                   // https://linux.die.net/man/1/avr-g++
                                        "-w -x c++ " +                //
                                        "-E " +                       // Preprocess only
                                        "-MMD " +                     // Generate dependencies to Sketch.inc
                                        "-MF *[TDIR]*Sketch.inc " +   //   " "
                                        "-DF_CPU=*[CLOCK]* " +        // Create #define for F_CPU
                                        "-mmcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                        "-DARDUINO_ARCH_AVR " +       // #define ARDUINO_ARCH_AVR
                                        "*[TDIR]**[IFILE]* ";         // Source file is temp/IFILE.x

  private static final String compCpp = "avr-g++ " +                  // https://linux.die.net/man/1/avr-g++
                                        "-c " +                       // Compile but do not link
                                        "-g " +                       // Enable link-time optimization
                                        "-Os " +                      // Optimize for size
                                        "-w " +                       // Inhibit all warning messages.
                                        "-std=gnu++11 " +             // Support GNU extensions to C++
                                        "-fpermissive " +             // Downgrade nonconformant code errors to warnings
                                        "-fno-exceptions " +          // Disable exception-handling code
                                        "-ffunction-sections " +      // Separate functions in output file
                                        "-fdata-sections " +          // Separate data in output file
                                        "-fno-threadsafe-statics " +  // No extra code for C++ ABI routines
                                        "-flto " +                    // Run standard link optimizer (requires 5.4.0)
                                        "-DLTO_ENABLED " +
                                        "-DF_CPU=*[CLOCK]* " +        // Create #define for F_CPU
                                        "-mmcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                        "-DARDUINO_ARCH_AVR " +       // #define ARDUINO_ARCH_AVR
                                        "*[DEFINES]* " +              // Add in conditional #defines, if any
                                        "-MMD " +                     // Mention only user header files
                                        "-I *[TDIR]* " +              // Also search in temp directory for header files
                                        "*[TDIR]**[IFILE]* " +        // Source file is temp/IFILE.x
                                        "-o *[TDIR]**[IFILE]*.o ";    // Output to file temp/IFILE.x.o

    private static final String compC = "avr-gcc " +                  // https://linux.die.net/man/1/avr-gcc
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
                                        "-MMD " +                     //
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

  private static final String link = "avr-g++ " +                     // https://linux.die.net/man/1/avr-g++
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
                                        "-S " +                       // Display source  code  intermixed  with  disassembly
                                        "-t " +                       // Print the symbol table entries
                                        "*[TDIR]*Sketch.elf";         // Input file

  private static final String tohex = "avr-objcopy " +                // https://linux.die.net/man/1/avr-objcopy
                                        "-j .text " +                 // Copy .text section
                                        "-j .data " +                 // Copy .data section
                                        "-O ihex " +                  // Output format is Intel HEX
                                        "*[TDIR]*Sketch.elf " +       // Input file
                                        "*[TDIR]*Sketch.hex";         // Output file

  private static final String size = "avr-size " +                    // https://linux.die.net/man/1/avr-size
                                        "--format=avr " +             // Architecture is AVR
                                        "--mcu=*[CHIP]* " +           // Select CHIP microcontroller type
                                       "*[TDIR]*Sketch.elf";          // Input file

  private static String[][] asm = {
    {"COMP1", "avr-as -mmcu=*[CHIP]* -I *[IDIR]* *[TDIR]*Sketch.S -o *[TDIR]*Sketch.o "},
    {"COMP2", "avr-ld -mavrtiny *[TDIR]*Sketch.o -o *[TDIR]*Sketch.elf "},
    {"TOHEX", tohex},
    {"LST",   list},
    {"SIZE",  size},
  };
  private static String[][] build = {
    {"TOHEX", tohex},
    {"LST",   list},   // Note add "-l' for source path and line numbers (Warning: large lines!)
    {"SIZE",  size},
  };
  private static Map<String,Integer>  fuses = new HashMap<>();

  static {
    // Define fuse bits
    fuses.put("ckout", 4);    // System Clock Output
    fuses.put("wdton", 2);    // Watchdog Timer Always On
    fuses.put("rstdisbl", 1); // External Reset Disable
  }

  private static void removeFiles (String tmpDir) {
    // Remove any prior tmp files
    final File[] files = (new File(tmpDir)).listFiles();
    if (files != null) {
      for (File file : files) {
        if (!file.isDirectory()) {
          file.delete();
        }
      }
    }
  }

  static Map<String,String> compile (String src, Map<String,String> tags, JFrame tinyIde) throws Exception {
    String tmpDir = tags.get("TDIR");
    String tmpExe = tags.get("TEXE");
    String srcName = tags.get("FNAME").toLowerCase();
    removeFiles(tmpDir);
    boolean doAsm = srcName.endsWith(".s");
    boolean preOnly = (srcName.endsWith(".c") || srcName.endsWith(".cpp")) && "PREONLY".equals(tags.get("PREPROCESS"));
    boolean genProto = (srcName.endsWith(".c") || srcName.endsWith(".cpp")) && "GENPROTOS".equals(tags.get("PREPROCESS"));
    byte fuseBits = 0x0F;
    String clock = null;
    String chip = "attiny10";
    boolean arduino = false;
    Map<String,String> libraries = new HashMap<>();
    StringBuilder defines = new StringBuilder();
    Map<String,String> out = new HashMap<>();
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
          }
        }
      } else if (line.startsWith("#include")) {
        LastIncludeLine = Math.max(LastIncludeLine, lineNum);
        if (line.toLowerCase().contains("\"arduino.h\"")) {
          arduino = true;
        } else {
          Matcher mat = libMatch.matcher(line);
          if (mat.find()) {
            // Build list of #include <lib.h> references
            String libName = mat.group(1).toLowerCase();
            libName = libName.contains(".") ? libName.substring(0, libName.indexOf(".")) : libName;
            libraries.put(libName, libName);
          }
        }
      }
    }
    tags.put("CHIP", chip);
    tags.put("CLOCK", clock != null ? clock : "8000000");
    tags.put("DEFINES", defines.toString());
    // Build list of files we need to compile and link
    List<String> compFiles = new ArrayList<>();
    ATTinyC.ChipInfo chipInfo = ATTinyC.progProtocol.get(chip.toLowerCase());
    if ("TPI".equals(chipInfo.prog)) {
      out.put("INFO", "chip: " + chip + ", clock: " + tags.get("CLOCK") + ", fuses: " + Utility.hexChar(fuseBits));
    } else {
      out.put("INFO", "chip: " + chip + ", clock: " + tags.get("CLOCK") + ", lfuse: " + out.get("LFUSE") +
          ", hfuse: " + out.get("HFUSE") +", efuse: " + out.get("EFUSE"));
    }
    ATTinyC.ProgressBar progress = null;
    try {
      // Copy contents of "source" pane to temp file with appropriate extension for code type
      String mainFile = doAsm ? "Sketch.S" : "Sketch.cpp";
      compFiles.add(mainFile);
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
        // Copy selected resource files listed in "arduinofiles.txt" into tmpDir so compiler can reference them
        String cores = Utility.getFile("res:arduinocore.txt");
        String[] coreList = cores.split("\n");
        for (String core : coreList) {
          core = core.trim();
          if (core.startsWith("//")) {
            continue;
          }
          String fName = (new File(core)).getName();
          String fPath = (new File(core)).getParent();
          if (arduino && (chipInfo.lib.matches(fPath) || (chipInfo.useCore && "tinyCore".matches(fPath)))) {
            Utility.copyResourceToDir(core, tmpDir);
            if (fName.toLowerCase().endsWith(".c") || fName.toLowerCase().endsWith(".cpp") || fName.toLowerCase().endsWith(".s")) {
              compFiles.add(fName);
            }
          }
        }
        // Copy over header and code files for any libraries referenced
        String libs = Utility.getFile("res:arduinolib.txt");
        String[] libList = libs.split("\n");
        for (String lib : libList) {
          lib = lib.trim();
          if (lib.startsWith("//")) {
            continue;
          }
          String fName = (new File(lib)).getName();
          String fPath = (new File(lib)).getParent();
          if (fPath != null && libraries.containsKey(fPath.toLowerCase())) {
            Utility.copyResourceToDir("libraries" + fileSep + lib, tmpDir);
            if (fName.toLowerCase().endsWith(".c") || fName.toLowerCase().endsWith(".cpp") || fName.toLowerCase().endsWith(".s")) {
              compFiles.add(fName);
            }
          }
        }
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
            String pattern = "#\\s\\d+\\s\"(.*?)\"";
            Pattern lMatch = Pattern.compile(pattern);
            while (lines.hasMoreElements()) {
              String line = lines.nextToken();
              Matcher mat = lMatch.matcher(line);
              if (mat.find()) {
                String seq = mat.group(1);
                inSketch = seq.matches(tmpDir + "Sketch.cpp");
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
        progress = new ATTinyC.ProgressBar(tinyIde, "Compiling and Building");
        progress.setMaximum(compFiles.size());
        int progCount = 0;
        progress.setValue(progCount);
        // Compile all source files
        StringBuilder linkList = new StringBuilder();
        for (String compFile : compFiles) {
          String suffix = compFile.substring(compFile.indexOf("."));
          tags.put("IFILE", compFile);
          linkList.append(tmpDir).append(compFile).append(".o ");
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
    return out;
  }
}
