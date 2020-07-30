import java.io.*;
import java.math.BigInteger;
import java.util.*;

/**
 *  Homebrew, One Pass, Atmel-style Asssember for ATTiny4,5,9,10 Family
 *  Author: Wayne Holder (https://sites.google.com/site/wayneholder/attiny-4-5-9-10-assembly-ide-and-programmer)
 *  License: MIT (https://opensource.org/licenses/MIT)
 */

class ATTiny10Assembler implements Serializable {
  private static char[]              hex = {'0', '1', '2', '3', '4', '5', '6', '7',
                                            '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
  private static Map<String,Integer>  regToReg =  new HashMap<>();
  private static Map<String,Integer>  noOperand = new HashMap<>();
  private static Map<String,Integer>  bitToReg1 = new HashMap<>();
  private static Map<String,Integer>  bitToReg2 = new HashMap<>();
  private static Map<String,Integer>  oneReg1 = new HashMap<>();
  private static Map<String,Integer>  oneReg2 = new HashMap<>();
  private static Map<String,Integer>  branch =  new HashMap<>();
  private static Map<String,Integer>  zcodes =  new HashMap<>();
  private static Map<String,Integer>  immedi6 = new HashMap<>();
  private static Map<String,Integer>  immedi8 = new HashMap<>();
  private static Map<String,Integer>  ldInst =  new HashMap<>();
  private static Map<String,Integer>  stInst =  new HashMap<>();
  private static Map<String,Integer>  regPair = new HashMap<>();
  private static Map<String,Integer>  fuses = new HashMap<>();
  private byte[]                      output = new byte[1024];
  private Map<Integer,String>         listing = new HashMap<>();
  private Map<Integer,List<String>>   commentLines = new HashMap<>();
  private int                         codeAdd = 0;
  private int                         maxAdd = 0;
  private int                         dataAdd = 0x40;
  private boolean                     cSeg;
  private Map<String,Object>          symbols = new TreeMap<>();
  private List<Inst>                  pass2 = new ArrayList<>();
  private byte                        fuseBits = (byte) 0xFF;

  {
    Arrays.fill(output, (byte) 0xFF);
  }

  static {
    // Define register to register opcodes (---- --sd dddd ssss)
    regToReg.put("adc",    0x1C00);     // Add with Carry
    regToReg.put("add",    0x0C00);     // Add without Carry
    regToReg.put("and",    0x2000);     // Logical AND
    regToReg.put("cp",     0x1400);     // Compare
    regToReg.put("cpc",    0x0400);     // Compare with Carry
    regToReg.put("cpse",   0x1000);     // Compare Skip if Equal
    regToReg.put("eor",    0x2400);     // Exclusive OR
    regToReg.put("mov",    0x2C00);     // Copy Register
    regToReg.put("or",     0x2800);     // Logical OR
    regToReg.put("sbc",    0x0800);     // Subtract with Carry
    regToReg.put("sub",    0x1800);     // Subtract without Carry
    // Define no operand opcodes
    noOperand.put("break", 0x9598);     // Break
    noOperand.put("clc",   0x9488);     // Clear Carry Flag
    noOperand.put("clh",   0x94D8);     // Clear Half Carry Flag
    noOperand.put("cli",   0x94F8);     // Clear Global Interrupt Flag
    noOperand.put("cln",   0x94A8);     // Clear Negative Flag
    noOperand.put("cls",   0x94C8);     // Clear Signed Flag
    noOperand.put("clt",   0x94E8);     // Clear T Flag
    noOperand.put("clv",   0x94B8);     // Clear Overflow Flag
    noOperand.put("clz",   0x9498);     // Clear Zero Flag
    noOperand.put("eicall",0x9519);     // Extended Indirect Call to Subroutine
    noOperand.put("eijmp", 0x9419);     // Extended Indirect Jump
    noOperand.put("icall", 0x9509);     // Indirect Call to Subroutine
    noOperand.put("ijmp",  0x9409);     // Indirect Jump
    noOperand.put("nop",   0x0000);     // No Operation
    noOperand.put("ret",   0x9508);     // Return from Subroutine
    noOperand.put("reti",  0x9518);     // Return from Interrupt
    noOperand.put("sec",   0x9408);     // Set Carry Flag
    noOperand.put("seh",   0x9458);     // Set Half Carry Flag
    noOperand.put("sei",   0x9478);     // Set Global Interrupt Flag
    noOperand.put("sen",   0x9428);     // Set Negative Flag
    noOperand.put("ses",   0x9448);     // Set Signed Flag
    noOperand.put("set",   0x9468);     // Set T Flag
    noOperand.put("sev",   0x9438);     // Set Overflow Flag
    noOperand.put("sez",   0x9418);     // Set Zero Flag
    noOperand.put("sleep", 0x9588);     // Sleep
    noOperand.put("wdr",   0x95A8);     // Watchdog Reset
    // Define bit to register codes of form (---- ---- rrrr rbbb)
    bitToReg1.put("cbi",   0x9800);     // Clear Bit in I/O Register
    bitToReg1.put("sbi",   0x9A00);     // Set Bit in I/O Register
    bitToReg1.put("sbic",  0x9900);     // Skip if Bit in I/O Register is Cleared
    bitToReg1.put("sbis",  0x9B00);     // Skip if Bit in I/O Register is Set
    // Define bit to register codes of form (---- ---r rrrr 0bbb)
    bitToReg2.put("bst",   0xFA00);     // Bit Store from Bit in Register to T Flag in SREG
    bitToReg2.put("bld",   0xF800);     // Bit Load from the T Flag in SREG to a Bit in Register
    bitToReg2.put("sbrc",  0xFC00);     // Skip if Bit in Register is Cleared
    bitToReg2.put("sbrs",  0xFE00);     // Skip if Bit in Register is Set
    // Define one register operand opcodes (---- ---r rrrr ----)
    oneReg1.put("pop",     0x900F);     // Pop Register from Stack
    oneReg1.put("push",    0x920F);     // Push Register on Stack
    oneReg1.put("asr",     0x9405);     // Arithmetic Shift Right
    oneReg1.put("com",     0x9400);     // One’s Complement
    oneReg1.put("dec",     0x940A);     // Decrement
    oneReg1.put("inc",     0x9403);     // Increment
    oneReg1.put("lsr",     0x9406);     // Logical Shift Right
    oneReg1.put("neg",     0x9401);     // Two’s Complement
    oneReg1.put("ror",     0x9407);     // Rotate Right through Carry
    oneReg1.put("swap",    0x9402);     // Swap Nibbles
    // Special reg to reg instructions where reg used twice
    oneReg1.put("rol",     0x1C00);     // Rotate Left trough Carry, Note alias for adc r,r
    oneReg1.put("clr",     0x2400);     // Clear Register, Note: alias for eor r,r
    oneReg1.put("tst",     0x2000);     // Test for Zero or Minus, Note: Logical and r,r
    oneReg1.put("lsl",     0x0C00);     // Logical Shift Left, alias for add r,r
    // Define one register operand opcodes (---- ---- rrrr ----) Note: r16-r31 only
    oneReg2.put("ser",     0xEF0F);     // Set all Bits in Register
    // Branch relative opcodes (---- --kk kkkk k---)
    branch.put("brcc",     0xF400);     // Branch if Carry Cleared
    branch.put("brcs",     0xF000);     // Branch if Carry Set
    branch.put("breq",     0xF001);     // Branch if Equal
    branch.put("brge",     0xF404);     // Branch if Greater or Equal (Signed)
    branch.put("brhc",     0xF405);     // Branch if Half Carry Flag is Cleared
    branch.put("brhs",     0xF005);     // Branch if Half Carry Flag is Set
    branch.put("brid",     0xF407);     // Branch if Global Interrupt is Disabled
    branch.put("brie",     0xF007);     // Branch if Global Interrupt is Enabled
    branch.put("brlo",     0xF000);     // Branch if Lower (Unsigned) Note: synonym for 'brcs'
    branch.put("brlt",     0xF004);     // Branch if Less Than (Signed)
    branch.put("brmi",     0xF002);     // Branch if Minus
    branch.put("brne",     0xF401);     // Branch if Not Equal
    branch.put("brpl",     0xF402);     // Branch if Plus
    branch.put("brsh",     0xF400);     // Branch if Same or Higher (Unsigned)
    branch.put("brtc",     0xF406);     // Branch if the T  Flag is Cleared
    branch.put("brts",     0xF006);     // Branch if the T Flag is Set
    branch.put("brvc",     0xF403);     // Branch if Overflow Cleared
    branch.put("brvs",     0xF003);     // Branch if Overflow Set
    branch.put("rjmp",     0xC000);     // Relative Jump
    branch.put("rcall",    0xD000);     // Relative Call to Subroutine
    // Z Register related opcodes (---- ---r rrrr ----)
    zcodes.put("xch",      0x9204);     //  Exchange
    zcodes.put("las",      0x9205);     //  Load And Set
    zcodes.put("lat",      0x9207);     //  Load And Toggle
    zcodes.put("lac",      0x9206);     //  Load And Clear
    // 8 bit Immediate instructions (---- kkkk rrrr kkkk) Note: r16-r31 only)
    immedi8.put("andi",    0x7000);     // Logical AND with Immediate
    immedi8.put("cpi",     0x3000);     // Compare with Immediate
    immedi8.put("ldi",     0xE000);     // Load Immediate
    immedi8.put("ori",     0x6000);     // Logical OR with Immediate
    immedi8.put("sbci",    0x4000);     // Subtract Immediate with Carry
    immedi8.put("subi",    0x5000);     // Subtract Immediate
    immedi8.put("sbr",     0x6000);     // Set Bits in Register (synonym for ori)
    immedi8.put("cbr",     0x7000);     // Clear Bits in Register (synonym for andi)
    // 6 bit Immediate instructions (---- ---- kkdd kkkk) Note: upper 4 register pairs (r25:524, XH:XL, YH:YL, ZH:ZL)
    immedi6.put("adiw",    0x9600);     // Add Immediate to Word
    immedi6.put("sbiw",    0x9700);     // Subtract Immediate from Word
    // Define ld and st X/Y/Z-based instructions (---- ---r rrrr ----)
    ldInst.put("x",        0x900C);
    ldInst.put("x+",       0x900D);
    ldInst.put("-x",       0x900E);
    ldInst.put("y",        0x8008);
    ldInst.put("y+",       0x9009);
    ldInst.put("-y",       0x900A);
    ldInst.put("z",        0x8000);
    ldInst.put("z+",       0x9001);
    ldInst.put("-z",       0x9002);

    stInst.put("x",        0x920C);
    stInst.put("x+",       0x920D);
    stInst.put("-x",       0x920E);
    stInst.put("y",        0x8208);
    stInst.put("y+",       0x9209);
    stInst.put("-y",       0x920A);
    stInst.put("z",        0x8200);
    stInst.put("z+",       0x9201);
    stInst.put("-z",       0x9202);
    // Define register pair and register alias values
    regPair.put("r25:r24", 0);
    regPair.put("r27:r26", 1);
    regPair.put("r29:r28", 2);
    regPair.put("r31:r30", 3);
    regPair.put("xh:xl",   1);
    regPair.put("yh:yl",   2);
    regPair.put("zh:zl",   3);
    regPair.put("zh",     31);
    regPair.put("zl",     30);
    regPair.put("yh",     29);
    regPair.put("yl",     28);
    regPair.put("xh",     27);
    regPair.put("xl",     26);
    // Define fuse bits
    fuses.put("ckout",     4);     // System Clock Output
    fuses.put("wdton",     2);     // Watchdog Timer Always On
    fuses.put("rstdisbl",  1);     // External Reset Disable
  }

  interface Inst {
    void emit ();
  }

  private class Branch implements Inst, Serializable {
    private String[]  parts;
    private String    comment;
    private int       add;

    Branch (String[] parts, int add, String comment) {
      this.parts = parts;
      this.add = add;
      this.comment = comment;
    }

    public void emit () {
      try {
        String op = parts[0];
        String arg = parts[1];
        int loc;
        if (arg.toLowerCase().startsWith("pc")) {
          arg = arg.substring(2);
          if (arg.startsWith("+"))
            arg = arg.substring(1);
          int rOff = arg.length() > 0 ? Integer.parseInt(arg) : 0;
          loc = add + rOff;
        } else {
          try {
            loc = regValue(arg);
          } catch (Exception ex) {
            addCommentOrError(add, "* * * Err: " + op + " " + arg);
            loc = 0;
          }
        }
        int inst = branch.get(op);
        int off = loc - add - 1;
        if ("rjmp".equals(op)  ||  "rcall".equals(op)) {
          // ---- kkkk kkkk kkkk
          emitCode(comment, add, parts, inst + (off & 0xFFF));
        } else {
          // ---- --kk kkkk k---
          emitCode(comment, add, parts, inst + ((off & 0x7F) << 3));
        }
      } catch (Exception ex) {
        addCommentOrError(add, " * * * " + ex.toString() + " " + parts[0]  + " " + parts[1]);
      }
    }
  }

  private class Type3 implements Inst, Serializable {
    private String[]  parts;
    private String    comment;
    private int       add;

    Type3 (String[] parts, int add, String comment) {
      this.parts = parts;
      this.add = add;
      this.comment = comment;
    }

    public void emit () {
      try {
        String op = parts[0].toLowerCase();
        int dReg = "z".equals(parts[1].toLowerCase()) ? 0 : regValue(parts[1]);
        int sReg = regValue(parts[2]);
        if (regToReg.containsKey(op)) {
          int inst = regToReg.get(op);
          // ---- --sd dddd ssss
          emitCode(comment, add, parts, inst + (dReg << 4) + (sReg & 0x0F) + ((sReg & 0x10) << 5));
        } else if (bitToReg1.containsKey(op)) {
          int inst = bitToReg1.get(op);
          // ---- ---- rrrr rbbb
          emitCode(comment, add, parts, inst + (dReg << 3) + (sReg & 0x07));
        } else if (bitToReg2.containsKey(op)) {
          int inst = bitToReg2.get(op);
          // ---- ---r rrrr 0bbb
          emitCode(comment, add, parts, inst + (dReg << 4) + (sReg & 0x07));
        } else if (zcodes.containsKey(op)) {
          int inst = zcodes.get(op);
          // ---- ---r rrrr ----
          emitCode(comment, add, parts, inst + (sReg << 4));
        } else if (immedi6.containsKey(op)) {
          int inst = immedi6.get(op);
          // ---- ---- kkdd kkkk Note: dd is upper 4 register pairs (r25:524, XH:XL, YH:YL, ZH:ZL)
          emitCode(comment, add, parts, inst + (dReg << 4) + ((sReg & 0x30) << 2) + (sReg & 0x0F));
        } else if (immedi8.containsKey(op)) {
          int inst = immedi8.get(op);
          // ---- kkkk rrrr kkkk
          emitCode(comment, add, parts, inst + ((dReg - 16) << 4) + ((sReg & 0xF0) << 4) + (sReg & 0x0F));
        } else if ("in".equals(op)) {
          // 1011 0aar rrrr aaaa
          emitCode(comment, add, parts, 0xB000 + (dReg << 4) + (sReg & 0x0F) + ((sReg & 0x30) << 5));
        } else if ("out".equals(op)) {
          // 1011 1aar rrrr aaaa
          emitCode(comment, add, parts, 0xB800 + (sReg << 4) + (dReg & 0x0F) + ((dReg & 0x30) << 5));
        } else if ("lds".equals(op)) {  // lds r,k
          // 1010 0kkk dddd kkkk  Note Bit order in 'a' bits is: ---- -546 ---- 3210
          emitCode(comment, add, parts, 0xA000 + ((dReg - 16) << 4) + (sReg & 0x0F) +
              ((sReg & 0x10) << 5) + ((sReg & 0x20) << 5) + ((sReg & 0x40) << 2));
        } else if ("sts".equals(op)) {  // sts k,r
          // 1010 1kkk dddd kkkk  Note Bit order in 'a' bits is: ---- -546 ---- 3210
          emitCode(comment, add, parts, 0xA800 + ((sReg - 16) << 4) + (dReg & 0x0F) +
              ((dReg & 0x10) << 5) + ((dReg & 0x20) << 5) + ((dReg & 0x40) << 2));
        } else if ("ld".equals(op)) {
          int inst = ldInst.get(parts[2].toLowerCase());
          // ---- ---r rrrr ----
          emitCode(comment, add, parts, inst + (dReg << 4));
        } else if ("st".equals(op)) {
          int inst = stInst.get(parts[1].toLowerCase());
          // ---- ---r rrrr ----
          emitCode(comment, add, parts, inst + (sReg << 4));
        } else {
          addCommentOrError(add, " * * * Unknown: " + op + " " + parts[1] + ", " + parts[2]);
        }
      } catch (Exception ex) {
        addCommentOrError(add, " * * * " + ex.toString() + parts[0]  + " " + parts[1] + ", " + parts[2]);
      }
    }
  }

  void assemble (String code) {
    StringTokenizer tok = new StringTokenizer(code, "\n\r");
    while (tok.hasMoreTokens()) {
      String line = tok.nextToken().trim();
      try {
        String comment = null;
        // Remove comment, if any
        int idx = line.indexOf(";");
        if (idx >= 0) {
          comment = line.substring(idx);
          line = line.substring(0, idx).trim();
        }
        idx = line.indexOf("//");
        if (idx >= 0) {
          comment = line.substring(idx);
          line = line.substring(1, idx + 1).trim();
        }
        String label;
        idx = line.indexOf(":");
        if (idx >= 0) {
          label = line.substring(0, idx);
          line = line.substring(idx + 1).trim();
          String key = label.toLowerCase();
          int val = cSeg ? codeAdd : dataAdd;
          symbols.put(key, val);
          if (cSeg)
            addCommentOrError(codeAdd, label + ":");
        }
        String[] parts = Utility.parse(line);
        if (parts.length > 0 && ".fuses".equals(parts[0])) {
          byte tmp = 0;
          for (int ii = 1; ii < parts.length; ii++) {
            tmp |= (byte) fuses.get(parts[ii]).intValue();
          }
          fuseBits = (byte) ~tmp;
        } else if (parts.length > 0 && ".db".equals(parts[0])) {
          if (cSeg) {
            int byteAdd = 0;
            int word = 0;
            boolean first = true;
            for (int ii = 1; ii < parts.length; ii++) {
              String tmp = parts[ii];
              int val = regValue(tmp);
              if (++byteAdd == 2) {
                emitCode(comment, codeAdd++, first ? parts : new String[0], word | ((val & 0xFF) << 8));
                first = false;
                word = 0;
                byteAdd = 0;
              } else {
                word = val & 0xFF;
              }
            }
            if (byteAdd > 0) {
              emitCode(comment, codeAdd++, first ? parts : new String[0], word);
            }
            maxAdd = Math.max(maxAdd, codeAdd << 1);
          } else {
            addCommentOrError(codeAdd, ".db directive doesn't work in DSEG");
          }
        } else if (parts.length > 0 && ".dw".equals(parts[0])) {
          if (cSeg) {
            boolean first = true;
            for (int ii = 1; ii < parts.length; ii++) {
              String tmp = parts[ii];
              int val = regValue(tmp);
              emitCode(comment, codeAdd++, first ? parts : new String[0], val & 0xFFFF);
              first = false;
            }
            maxAdd = Math.max(maxAdd, codeAdd << 1);
          } else {
            addCommentOrError(codeAdd, ".dw directive doesn't work in DSEG");
          }
        } else if (parts.length == 0  && comment != null) {
          addCommentOrError(codeAdd, comment);
        } else if (parts.length == 1) {
          // Process directives, such as .dseg
          String op = parts[0].toLowerCase();
          if (".dseg".equals(op)) {
            cSeg = false;
          } else if (".cseg".equals(op)) {
            cSeg = true;
          } else if (noOperand.containsKey(op)) {
            emitCode(comment, codeAdd++, parts, noOperand.get(op));
          } else {
            addCommentOrError(codeAdd, op);
          }
        } else if (parts.length == 2) {
          // process directives and opcodes with one argument
          String op = parts[0].toLowerCase();
          String arg = parts[1];
          if (".org".equals(op)) {
            // Set origin address
            if (cSeg) {
              codeAdd = regValue(arg);
            } else {
              dataAdd = regValue(arg);
            }
            addCommentOrError(codeAdd, ".org " + arg);
          } else if (".byte".equals(op)) {
            // Allocate data space
            dataAdd += regValue(arg);
          } else if (".device".equals(op)) {
            // Set device type
            try {
              Properties deviceSymbols = Utility.getResourceMap(arg.toLowerCase() + ".props");
              // Copy device symbols into symbol table
              Enumeration ee = deviceSymbols.propertyNames();
              while (ee.hasMoreElements()) {
                String key = (String) ee.nextElement();
                String val = deviceSymbols.getProperty(key);
                try {
                  symbols.put(key.toLowerCase(), toNum(val));
                } catch (NumberFormatException ex) {
                  addCommentOrError(codeAdd, "Bad device symbol: " + key + ": " + val);
                }
              }
            } catch (IOException ex) {
              addCommentOrError(codeAdd, "unknown device type: " + arg);
            }
          } else if (".equ".equals(op) || ".eq".equals(op) || ".def".equals(op)) {
            // Process equate
            String[] tmp = parts[1].toLowerCase().split("=");
            if (tmp.length == 2) {
              String name = tmp[0].trim();
              int value = regValue(tmp[1].trim());
              symbols.put(name, value);
            }
          } else if (oneReg1.containsKey(op)) {
            int reg = regValue(arg);
            int inst = oneReg1.get(op);
            if ("rol".equals(op)  ||  "lsl".equals(op)  ||  "clr".equals(op)  ||  "tst".equals(op)) {
              // ---- --sd dddd ssss
              emitCode(comment, codeAdd++, parts, inst + (reg << 4) + (reg & 0x0F) + ((reg & 0x10) << 5));
            } else {
              // ---- ---r rrrr ----
              emitCode(comment, codeAdd++, parts, inst + (reg << 4));
            }
          } else if (oneReg2.containsKey(op)) {
            int reg = regValue(arg);
            int inst = oneReg2.get(op);
            // ---- ---- rrrr ----
            emitCode(comment, codeAdd++, parts, inst + ((reg - 16) << 4));
          } else if ("bset".equals(op)  ||  "bclr".equals(op)) {
            int bit = regValue(arg);
            int inst = "bset".equals(op) ? 0x9408 : 0x9488;
            // ---- ---- -bbb ----
            emitCode(comment, codeAdd++, parts, inst + (bit << 4));
          } else if (branch.containsKey(op)) {
            pass2.add(new Branch(parts, codeAdd++, comment));
          } else {
            addCommentOrError(codeAdd, "Unknown: " + op + " " + arg);
          }
        } else if (parts.length == 3) {
          // process two argument opcodes
          pass2.add(new Type3(parts, codeAdd++, comment));
        }
      } catch (Exception ex) {
        addCommentOrError(codeAdd, "* * * Err: " + line + " - " + ex.getMessage());
        codeAdd++;
        maxAdd = codeAdd << 1;
      }
    }
    // Emit all relative branches
    for (Inst inst : pass2) {
      inst.emit();
    }
  }

  private void addCommentOrError (int addr, String txt) {
    List<String> cList = commentLines.computeIfAbsent(addr, k -> new ArrayList<>());
    cList.add(txt);
  }

  String getHex () {
    StringBuilder buf = new StringBuilder();
    buf.append(":020000020000FC\n");  // Set origin at 0
    buf.append("*");
    buf.append(hexChar(fuseBits));
    buf.append("\n");
    byte crc = 0;
    boolean dirty = false;
    for (int ii = 0; ii < maxAdd; ii++) {
      if ((ii & 0x0F) == 0) {
        byte len = (byte) Math.min(maxAdd - ii, 16);
        crc = len;
        crc += ii & 0xFF;
        crc += (ii >> 8) & 0xFF;
        buf.append(':');
        buf.append(byteToHex(len));
        buf.append(intToHex(ii));
        buf.append("00");
      }
      byte data = output[ii];
      buf.append(byteToHex(data));
      crc += data;
      dirty = true;
      if ((ii & 0x0F) == 0x0F) {
        buf.append(byteToHex((byte) ((~crc) + 1)));
        buf.append("\n");
        dirty = false;
      }
    }
    if (dirty) {
      buf.append(byteToHex((byte) ((~crc) + 1)));
      buf.append("\n");
    }
    buf.append(":00000001FF\n");    // End of file
    return buf.toString();
  }

  private int toNum (String val) {
    return ((BigInteger) ExpressionParser.eval(ExpressionParser.parse(val, null), symbols)).intValue();
  }

  String getListing () {
    StringBuilder buf = new StringBuilder("Fuses: 0x" + byteToHex(fuseBits) + "\n");
    for (int ii = 0; ii < maxAdd >> 1; ii++) {
      List<String> cLines = commentLines.get(ii);
      if (cLines != null) {
        for (String cLine : cLines) {
          buf.append(cLine);
          buf.append("\n");
        }
      }
      if (listing.get(ii) != null) {
        buf.append(listing.get(ii));
        buf.append("\n");
      }
    }
    return buf.toString();
  }

  private void emitCode (String comment, int address, String[] parts, int inst) {
    int byteAdd = address << 1;
    output[byteAdd++] = (byte) (inst & 0xFF);
    output[byteAdd++] = (byte) (inst >> 8);
    maxAdd = Math.max(maxAdd, byteAdd);
    StringBuilder buf = new StringBuilder();
    buf.append(intToHex(address));
    buf.append(": ");
    buf.append(intToHex(inst));
    buf.append("  ");
    boolean first = true;
    for (String arg : parts) {
      buf.append(first ? "" : " ").append(arg);
      first = false;
    }
    if (comment != null) {
      while (buf.length() < 32)
        buf.append(" ");
      buf.append(comment);
    }
    listing.put(address, buf.toString());
  }


  private int regValue (String reg) {
    reg = reg.toLowerCase();
    if (symbols.containsKey(reg)) {
      Object val = symbols.get(reg);
      if (val instanceof Number) {
        return ((Number) val).intValue();
      } else {
        throw new IllegalStateException("Value '" + reg + "'" + " not a number");
      }
    }
    if (regPair.containsKey(reg)) {
      return regPair.get(reg);
    }
    if (stInst.containsKey(reg)  ||  ldInst.containsKey(reg)) {
      return 0;
    }
    try {
      return toNum(reg);
    } catch (NumberFormatException ex) {
      throw new IllegalStateException("Unknown symbol '" + reg + "'");
    }
  }

  private static char hexChar (byte val) {
    return hex[val & 0x0F];
  }

  private String byteToHex (byte val) {
    return "" + hex[(val >> 4) & 0x0F] + hex[val & 0x0F];
  }

  private static String intToHex (int val) {
    return "" + hex[(val >> 12) & 0x0F] + hex[(val >> 8) & 0x0F] + hex[(val >> 4) & 0x0F] + hex[val & 0x0F];
  }
}
