import java.awt.*;
import java.awt.event.*;

import java.io.*;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.channels.ReadableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.*;
import java.util.prefs.Preferences;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import javax.swing.*;
import javax.swing.filechooser.FileNameExtensionFilter;

import jssc.SerialNativeInterface;

import static javax.swing.JOptionPane.INFORMATION_MESSAGE;
import static javax.swing.JOptionPane.showMessageDialog;

/**
   *  An IDE for ATTiny10 Series MIcrocontrolles
   *  Author: Wayne Holder, 2017
   *  License: MIT (https://opensource.org/licenses/MIT)
   *
   *  TBD:
   *    Convert to JSSC for Serial IO [done]
   *    Make it run on Windows [done]
   *    Make it run on Linux
   *    Switch from CrossPack-AVR toolchain to one extracted from Arduino
   *    Implement Arduino-like I/O functions (pinMode(), analogWrite(), etc.)
   *    Program and debug? See: http://www.ruemohr.org/docs/debugwire.html
   *    https://github.com/dcwbrown/dwire-debug/blob/master/src/commandline/commandline.c
   */

public class ATTinyC extends JFrame implements JSSCPort.RXEvent {
  private static final String       VERSION = "1.0 beta";
  private static final String       fileSep =  System.getProperty("file.separator");
  private static boolean            windows = System.getProperty("os.name").toLowerCase().contains("win");
  private static Font               tFont = new Font(windows ? "Consolas" : "Menlo", Font.PLAIN, 12);
  private static int                cmdMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
  private static KeyStroke          OPEN_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_O, cmdMask) ;
  private static KeyStroke          SAVE_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_S, cmdMask) ;
  private static KeyStroke          QUIT_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_Q, cmdMask) ;
  private static KeyStroke          BUILD_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_B, cmdMask) ;
  static Map<String,ChipInfo>       progProtocol = new HashMap<>();
  private enum                      Tab {SRC(0), LIST(1), HEX(2), PROG(3), INFO(4); final int num; Tab(int num) {this.num = num;}}
  private String                    osName = System.getProperty("os.name").toLowerCase();
  private JTabbedPane               tabPane;
  private JFileChooser              fc = new JFileChooser();
  private CodeEditPane              codePane;
  private JTextArea                 listPane, hexPane, progPane, infoPane;
  private JMenuItem                 saveMenu;
  private String                    tmpDir, tmpExe, chip, editFile;
  private boolean                   directHex, compiled, codeDirty;
  private File                      cFile;
  private transient Preferences     prefs = Preferences.userRoot().node(this.getClass().getName());
  private String                    icspProgrammer = prefs.get("icsp_programmer", "avrisp2");
  private transient JSSCPort        jPort;
  private Map<String, String>       compileMap;

  static class ChipInfo {
    String  prog, lib, fuses;
    boolean useCore;

    ChipInfo (String prog, String lib, String fuses, boolean useCore) {
      this.prog = prog;
      this.lib = lib;
      this.fuses = fuses;
      this.useCore = useCore;
    }
  }

  static {
    // List of ATtiny types and the Protocol used to Program them.
    progProtocol.put("attiny4",  new ChipInfo("TPI", "tiny10", "FF", false));
    progProtocol.put("attiny5",  new ChipInfo("TPI", "tiny10", "FF", false));
    progProtocol.put("attiny9",  new ChipInfo("TPI", "tiny10", "FF", false));
    progProtocol.put("attiny10", new ChipInfo("TPI", "tiny10", "FF", false));
    progProtocol.put("attiny24", new ChipInfo("ICSP", "tinyX4", "l:60,h:DF,e:FF", true));
    progProtocol.put("attiny44", new ChipInfo("ICSP", "tinyX4", "l:60,h:DF,e:FF", true));
    progProtocol.put("attiny84", new ChipInfo("ICSP", "tinyX4", "l:60,h:DF,e:FF", true));
    progProtocol.put("attiny25", new ChipInfo("ICSP", "tinyX5", "l:60,h:DF,e:FF", true));
    progProtocol.put("attiny45", new ChipInfo("ICSP", "tinyX5", "l:60,h:DF,e:FF", true));
    progProtocol.put("attiny85", new ChipInfo("ICSP", "tinyX5", "l:60,h:DF,e:FF", true));
  }

  {
    FileNameExtensionFilter[] filters = {
        new FileNameExtensionFilter("AVR .c files", "c"),
        new FileNameExtensionFilter("AVR .asm or .s files", "asm", "s"),
    };
    String ext = prefs.get("default.extension", "c");
    for (FileNameExtensionFilter filter : filters) {
      fc.addChoosableFileFilter(filter);
      if (filter.getExtensions()[0].equals(ext)) {
        fc.setFileFilter(filter);
      }
    }
    fc.setAcceptAllFileFilterUsed(true);
    fc.setMultiSelectionEnabled(false);
    fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
    prefs.putBoolean("enable_preprocessing", prefs.getBoolean("enable_preprocessing", false));
    prefs.putBoolean("gen_prototypes", prefs.getBoolean("gen_prototypes", false));
  }

  private void selectTab (Tab tab) {
    tabPane.setSelectedIndex(tab.num);
  }

  private void setDirtyIndicator (boolean dirty) {
    this.setTitle("ATTinyC: " + (editFile != null ? editFile : "") + (dirty ? " [unsaved]" : ""));
  }

  private void showAboutBox () {
    ImageIcon icon = null;
    try {
      icon = new ImageIcon(getClass().getResource("images/tiny10-128x128.jpg"));
    } catch (Exception ex) {
      ex.printStackTrace();
    }
    showMessageDialog(this,
      "By: Wayne Holder\n" +
        "tmpDir:  " + tmpDir + "\n" +
        "tmpExy:  " + tmpExe + "\n" +
        "Java Simple Serial Connector: " + SerialNativeInterface.getLibraryVersion() + "\n" +
        "JSSC Native Code DLL Version: " + SerialNativeInterface.getNativeLibraryVersion() + "\n",
      "ATtiny10IDE " + VERSION, INFORMATION_MESSAGE,  icon);
  }

  private void showPreferences () {
    ParmDialog.ParmItem[][] parmSet = {{
      new ParmDialog.ParmItem("Enable Preprocessing (Developer){*[PREPROCESS]*}", prefs.getBoolean("enable_preprocessing", true)),
      new ParmDialog.ParmItem("Generate Prototypes (Experimental){*[GEN_PROTOS]*}", prefs.getBoolean("gen_prototypes", true)),
    }};
    ParmDialog dialog = new ParmDialog(parmSet, null, new String[] {"Save", "Cancel"});
    dialog.setLocationRelativeTo(this);
    dialog.setVisible(true);              // Note: this call invokes dialog
    if (dialog.wasPressed()) {
      prefs.putBoolean("enable_preprocessing",  parmSet[0][0].value);
      prefs.putBoolean("gen_prototypes",        parmSet[0][1].value);
    }
  }

  private ATTinyC () {
    super("ATTinyC");
    setBackground(Color.white);
    setLayout(new BorderLayout(1, 1));
    // Create Tabbed Pane
    tabPane = new JTabbedPane();
    add("Center", tabPane);
    codePane = new CodeEditPane(prefs);
    codePane.setCodeChangeListener(() -> {
      setDirtyIndicator(codeDirty = true);
      compiled = false;
      listPane.setForeground(Color.red);
      hexPane.setForeground(Color.red);
    });
    tabPane.addTab("Source Code", null, codePane, "This is the editor pane where you enter source code");
    listPane = getScrollingTextPage(tabPane, "Listing", "Select this pane to view the assembler listing");
    hexPane = getScrollingTextPage(tabPane, "Hex Output", "Intel Hex Output file for programmer");
    progPane = getScrollingTextPage(tabPane, "Programmer", "Records communication with Arduino-based programmer");
    infoPane = getScrollingTextPage(tabPane, "Error Info", "Displays additional information about errors");
    infoPane.append("os.name: " + osName + "\n");
    // Add menu bar and menus
    JMenuBar menuBar = new JMenuBar();
    // Add "File" Menu
    JMenu fileMenu = new JMenu("File");
    JMenuItem mItem;
    fileMenu.add(mItem = new JMenuItem("About"));
    mItem.addActionListener(e -> showAboutBox());
    fileMenu.add(mItem = new JMenuItem("Preferences"));
    mItem.addActionListener(e -> showPreferences());
    fileMenu.addSeparator();
    fileMenu.add(mItem = new JMenuItem("New"));
    mItem.addActionListener(e -> {
      if (codePane.getText().length() == 0 || discardChanges()) {
        codePane.setForeground(Color.black);
        codePane.setText("");
        directHex = false;
        compiled = false;
        setDirtyIndicator(codeDirty = false);
        selectTab(Tab.SRC);
        cFile = null;
      }
    });
    fileMenu.add(mItem = new JMenuItem("Open"));
    mItem.setAccelerator(OPEN_KEY);
    mItem.addActionListener(e -> {
      fc.setSelectedFile(new File(prefs.get("default.dir", "/")));
      if (!codeDirty  ||  discardChanges()) {
        if (fc.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
          try {
            File oFile = fc.getSelectedFile();
            prefs.put("default.extension", ((FileNameExtensionFilter) fc.getFileFilter()).getExtensions()[0]);
            String tmp = Utility.getFile(oFile);
            if (tmp.contains(":00000001FF")) {
              if (!tmp.startsWith(":020000020000FC")) {
                tmp = ":020000020000FC\n" + tmp;
              }
              hexPane.setForeground(Color.black);
              hexPane.setText(tmp);
              selectTab(Tab.HEX);
              directHex = true;
            } else {
              cFile = oFile;
              codePane.setForeground(Color.black);
              codePane.setText(tmp);
              prefs.put("default.dir", editFile = oFile.getAbsolutePath());
              setDirtyIndicator(codeDirty = false);
              directHex = false;
              selectTab(Tab.SRC);
              saveMenu.setEnabled(true);
            }
            prefs.put("default.dir", oFile.getAbsolutePath());
          } catch (IOException ex) {
            ex.printStackTrace();
          }
        }
      }
    });
    fileMenu.add(saveMenu = new JMenuItem("Save"));
    saveMenu.setAccelerator(SAVE_KEY);
    saveMenu.setEnabled(false);
    saveMenu.addActionListener(e -> {
      Utility.saveFile(cFile, codePane.getText());
      setDirtyIndicator(codeDirty = false);
    });
    fileMenu.add(mItem = new JMenuItem("Save As..."));
    mItem.addActionListener(e -> {
      fc.setSelectedFile(new File(prefs.get("default.dir", "/")));
      if (fc.showSaveDialog(this) == JFileChooser.APPROVE_OPTION) {
        File sFile = fc.getSelectedFile();
        prefs.put("default.extension", ((FileNameExtensionFilter) fc.getFileFilter()).getExtensions()[0]);
        if (sFile.exists()) {
          if (doWarningDialog("Overwrite Existing file?")) {
            Utility.saveFile(sFile, codePane.getText());
          }
        } else {
          Utility.saveFile(sFile, codePane.getText());
        }
        cFile = sFile;
        prefs.put("default.dir", editFile = sFile.getAbsolutePath());
        setDirtyIndicator(codeDirty = false);
        saveMenu.setEnabled(true);
      }
    });
    fileMenu.add(mItem = new JMenuItem("Quit ATTiny10IDE"));
    mItem.setAccelerator(QUIT_KEY);
    mItem.addActionListener(e -> {
      if (!codeDirty  ||  discardChanges()) {
        if (jPort != null && jPort.isOpen()) {
          jPort.close();
        }
        System.exit(0);
      }
    });
    menuBar.add(fileMenu);
    // Add "Edit" Menu for undo/redo actions
    menuBar.add(codePane.getEditMenu());
    // Add "Actions" Menu
    JMenu actions = new JMenu("Actions");
    actions.add(mItem = new JMenuItem("Build"));
    mItem.setAccelerator(BUILD_KEY);
    mItem.addActionListener(e -> {
      if (cFile != null) {
        String fName = cFile.getName().toLowerCase();
        if (fName.endsWith(".asm")) {
          ATTiny10Assembler asm = new ATTiny10Assembler();
          asm.assemble(codePane.getText());
          listPane.setForeground(Color.black);
          listPane.setText(asm.getListing());
          hexPane.setForeground(Color.black);
          hexPane.setText(asm.getHex());
          compiled = true;
        } else {
          // Reinstall toolchain if there was an error last time we tried to build
          installToolchain(prefs.getBoolean("reload_toolchain", false));
          Thread cThread = new Thread(() -> {
            try {
              listPane.setForeground(Color.black);
              listPane.setText("");
              Map<String,String> tags = new HashMap<>();
              tags.put("TDIR", tmpDir);
              tags.put("TEXE", tmpExe);
              tags.put("IDIR", tmpExe + "avr" + fileSep + "include" + fileSep);
              tags.put("FNAME", fName);
              if (prefs.getBoolean("gen_prototypes", false)) {
                tags.put("PREPROCESS", "GENPROTOS");
              }
              compileMap = ATTinyCompiler.compile(codePane.getText(), tags, this);
              if (compileMap.containsKey("ERR")) {
                listPane.setForeground(Color.red);
                listPane.setText(compileMap.get("ERR"));
                //prefs.putBoolean("reload_toolchain", true);
                compiled = false;
              } else {
                listPane.setForeground(Color.black);
                listPane.setText(compileMap.get("INFO") + "\n\n" + compileMap.get("SIZE") + compileMap.get("LST"));
                hexPane.setForeground(Color.black);
                hexPane.setText(compileMap.get("HEX"));
                chip = compileMap.get("CHIP");
                compiled = true;
              }
            } catch (Exception ex) {
              prefs.putBoolean("reload_toolchain", true);
              ex.printStackTrace();
              infoPane.append("Stack Trace:\n");
              ByteArrayOutputStream bOut = new ByteArrayOutputStream();
              PrintStream pOut = new PrintStream(bOut);
              ex.printStackTrace(pOut);
              pOut.close();
              infoPane.append(bOut.toString() + "\n");
            }
          });
          cThread.start();
        }
        selectTab(Tab.LIST);
      } else {
        showErrorDialog("Please save file first!");
      }
    });
    JMenuItem preprocess = new JMenuItem("Run Preprocessor");
    actions.add(preprocess);
    prefs.addPreferenceChangeListener(evt -> preprocess.setVisible(prefs.getBoolean("enable_preprocessing", false)));
    preprocess.addActionListener(e -> {
      if (cFile != null) {
        String fName = cFile.getName().toLowerCase();
        if (fName.endsWith(".cpp") || fName.endsWith(".c")) {
          // Reinstall toolchain if there was an error last time we tried to build
          installToolchain(prefs.getBoolean("reload_toolchain", false));
          Thread cThread = new Thread(() -> {
            try {
              listPane.setForeground(Color.black);
              listPane.setText("");
              Map<String,String> tags = new HashMap<>();
              tags.put("TDIR", tmpDir);
              tags.put("TEXE", tmpExe);
              tags.put("IDIR", tmpExe + "avr" + fileSep + "include" + fileSep);
              tags.put("FNAME", fName);
              tags.put("PREPROCESS", "PREONLY");
              compileMap = ATTinyCompiler.compile(codePane.getText(), tags, this);
              if (compileMap.containsKey("ERR")) {
                listPane.setForeground(Color.red);
                listPane.setText(compileMap.get("ERR"));
                compiled = false;
              } else {
                listPane.setForeground(Color.black);
                listPane.setText(compileMap.get("PRE"));
              }
            } catch (Exception ex) {
              ex.printStackTrace();
              infoPane.append("Stack Trace:\n");
              ByteArrayOutputStream bOut = new ByteArrayOutputStream();
              PrintStream pOut = new PrintStream(bOut);
              ex.printStackTrace(pOut);
              pOut.close();
              infoPane.append(bOut.toString() + "\n");
            }
          });
          cThread.start();
        } else {
          listPane.setText("Must be .c or .cpp file");
        }
        selectTab(Tab.LIST);
      } else {
        showErrorDialog("Please save file first!");
      }
    });
    /*
     *    TPI Programmer Menus
     */
    JMenu tpiProg = new JMenu("TPI Programmer");
    actions.add(tpiProg);
    tpiProg.add(mItem = new JMenuItem("Program Device"));
    mItem.addActionListener(e -> {
      try {
        if (canProgram()) {
          String hex = hexPane.getText();
          String protocol = progProtocol.get(chip.toLowerCase()).prog;
          switch (protocol) {
          case "TPI":
            if (jPort != null && jPort.isOpen()) {
              selectTab(Tab.PROG);
              progPane.append("\nProgramming " + cFile.getName());
              jPort.sendString("\nD\n" + hex + "\n");
            } else {
              showErrorDialog("Serial port not selected!");
            }
            break;
          default:
            showErrorDialog("TPI Programming is not complatible with selected device");
            break;
          }
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Generate Arduino Programmer Code"));
    mItem.addActionListener(e -> {
      if (canProgram()) {
        String protocol = progProtocol.get(chip.toLowerCase()).prog;
        switch (protocol) {
        case "TPI":
          try {
            String genCode = Utility.getFile("res:ATTiny10GeneratedProgrammer.ino");
            String hex = hexPane.getText();
            CodeImage image = parseIntelHex(hex);
            byte[] code = image.data;
            StringBuilder buf = new StringBuilder();
            boolean first = true;
            for (int ii = 0; ii < code.length; ii++) {
              if ((ii & 0x0F) == 0) {
                buf.append(first ? "\n  " : ",\n  ");
                first = false;
              } else {
                buf.append(", ");
              }
              buf.append("0x");
              buf.append(Utility.hexChar((byte) (code[ii] >> 4)));
              buf.append(Utility.hexChar(code[ii]));
            }
            char fuseCode = Utility.hexChar(image.fuses);
            String outCode = genCode.replace("/*[CODE]*/", buf.toString()).replace("/*[FUSE]*/", Character.toString(fuseCode));
            outCode = outCode.replace("/*[NAME]*/", cFile.getName());
            String genFile = cFile.getAbsolutePath();
            int dot = genFile.lastIndexOf(".");
            if (dot > 0) {
              genFile = genFile.substring(0, dot) + "-prog.ino";
            }
            JFileChooser jFileChooser = new JFileChooser();
            jFileChooser.setSelectedFile(new File(genFile));
            if (jFileChooser.showSaveDialog(this) == JFileChooser.APPROVE_OPTION) {
              File sFile = jFileChooser.getSelectedFile();
              if (sFile.exists()) {
                if (doWarningDialog("Overwrite Existing file?")) {
                  Utility.saveFile(sFile, outCode);
                }
              } else {
                Utility.saveFile(sFile, outCode);
              }
            }
          } catch (Exception ex) {
            showErrorDialog(ex.getMessage());
          }
          break;
        case "ICSP":
          showErrorDialog("Protocol '" + protocol + " not currently supported");
          break;
        }
      }
    });
    tpiProg.add(mItem = new JMenuItem("Calibrate Clock"));
    mItem.addActionListener(e -> {
      try {
        if (jPort != null && jPort.isOpen()) {
          selectTab(Tab.PROG);
          String hex = Utility.getFile("res:clockcal.hex");
          progPane.append("\nProgramming Clock Code");
          jPort.sendString("\nD\n" + hex + "\n");
          jPort.sendString("M\n");
        } else {
          showErrorDialog("Serial port not selected!");
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Device Signature"));
    mItem.addActionListener(e -> {
      try {
        if (jPort != null && jPort.isOpen()) {
          selectTab(Tab.PROG);
          jPort.sendString("S\n");
        } else {
          showErrorDialog("Serial port not selected!");
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Power On"));
    mItem.addActionListener(e -> {
      try {
        jPort.sendString("V\n");

      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Power Off"));
    mItem.addActionListener(e -> {
      try {
        jPort.sendString("X\n");

      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    /*
     *    ICSP Programmer Menus
     */
    JMenu icpProg = new JMenu("ICSP Programmer");
    actions.add(icpProg);
    icpProg.add(mItem = new JMenuItem("Program Device"));
    mItem.addActionListener(e -> {
      try {
        if (canProgram()) {
          String hex = hexPane.getText();
          String protocol = progProtocol.get(chip.toLowerCase()).prog;
          switch (protocol) {
          case "ICSP":
            // Copy contents of "hex" pane to temp file with .hex extension
            selectTab(Tab.PROG);
            try {
              FileOutputStream fOut = new FileOutputStream(tmpDir + "code.hex");
              fOut.write(hex.getBytes(StandardCharsets.UTF_8));
              fOut.close();
            } catch (IOException ex) {
              progPane.append("\nError: " + ex.toString());
              break;
            }
            // Use AVRDUDE to program chip
            try {
              Map<String,String> tags = new HashMap<>();
              tags.put("PROG", icspProgrammer);
              tags.put("TDIR", tmpDir);
              tags.put("CHIP", chip);
              String exec = Utility.replaceTags("avrdude -Pusb -c *[PROG]* -p *[CHIP]* -U flash:w:*[TDIR]*code.hex", tags);
              String cmd = tmpExe + "bin" + System.getProperty("file.separator") + exec;
              System.out.println("Run: " + cmd);
              Process proc = Runtime.getRuntime().exec(cmd);
              String ret = Utility.runCmd(proc);
              int retVal = proc.waitFor();
              if (retVal != 0) {
                progPane.append("Error: " + ret + "\n");
              } else {
                progPane.append(ret + "\n");
              }
            } catch (IllegalStateException ex) {
              ex.printStackTrace();
            }
            break;
          default:
            showErrorDialog("ICSP Programming is not complatible with selected device");
            break;
         }
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    icpProg.add(mItem = new JMenuItem("Program Fuses"));
    mItem.addActionListener(e -> {
      ChipInfo chipInfo = progProtocol.get(chip.toLowerCase());
      try {
        if (canProgram()) {
          switch (chipInfo.prog) {
          case "TPI":
            selectTab(Tab.PROG);
            showErrorDialog("TPI not currently supported");
            break;
          case "ICSP":
            selectTab(Tab.PROG);
            progPane.setText("");
            Map<String,String> fuseDefaults = Utility.csvToMap(chipInfo.fuses);
            int lFuse = Integer.decode(Utility.choose(compileMap.get("PRAGMA.LFUSE"), "0x" + fuseDefaults.get("l")));
            int hFuse = Integer.decode(Utility.choose(compileMap.get("PRAGMA.HFUSE"), "0x" + fuseDefaults.get("h")));
            int eFuse = Integer.decode(Utility.choose(compileMap.get("PRAGMA.EFUSE"), "0x" + fuseDefaults.get("e")));
            ParmDialog.ParmItem[][] parmSet = {
              {
                new ParmDialog.ParmItem("CKDIV8{*[CKDIV8]*}",       !Utility.bit(lFuse, 7)),
                new ParmDialog.ParmItem("CKOUT{*[CKOUT]*}",         !Utility.bit(lFuse, 6)),
                new ParmDialog.ParmItem("SUT1{*[SUT]*}",            !Utility.bit(lFuse, 5)),
                new ParmDialog.ParmItem("SUT0{*[SUT]*}",            !Utility.bit(lFuse, 4)),
                new ParmDialog.ParmItem("CKSEL3{*[CKSEL]*}",        !Utility.bit(lFuse, 3)),
                new ParmDialog.ParmItem("CKSEL2{*[CKSEL]*}",        !Utility.bit(lFuse, 2)),
                new ParmDialog.ParmItem("CKSEL1{*[CKSEL]*}",        !Utility.bit(lFuse, 1)),
                new ParmDialog.ParmItem("CKSEL0{*[CKSEL]*}",        !Utility.bit(lFuse, 0)),
              }, {
                new ParmDialog.ParmItem("!RSTDISBL{*[RSTDISBL]*}",  !Utility.bit(hFuse, 7)),
                new ParmDialog.ParmItem("!DWEN{*[DWEN]*}",          !Utility.bit(hFuse, 6)),
                new ParmDialog.ParmItem("!SPIEN{*[SPIEN]*}",        !Utility.bit(hFuse, 5)),
                new ParmDialog.ParmItem("WDTON{*[WDTON]*}",         !Utility.bit(hFuse, 4)),
                new ParmDialog.ParmItem("EESAVE{*[EESAVE]*}",       !Utility.bit(hFuse, 3)),
                new ParmDialog.ParmItem("BODLEVEL2{*[BODLEVEL]*}",  !Utility.bit(hFuse, 2)),
                new ParmDialog.ParmItem("BODLEVEL1{*[BODLEVEL]*}",  !Utility.bit(hFuse, 1)),
                new ParmDialog.ParmItem("BODLEVEL0{*[BODLEVEL]*}",  !Utility.bit(hFuse, 0)),
            }, {
                new ParmDialog.ParmItem("*Not Used",                !Utility.bit(eFuse, 7)),
                new ParmDialog.ParmItem("*Not Used",                !Utility.bit(eFuse, 6)),
                new ParmDialog.ParmItem("*Not Used",                !Utility.bit(eFuse, 5)),
                new ParmDialog.ParmItem("*Not Used",                !Utility.bit(eFuse, 4)),
                new ParmDialog.ParmItem("*Not Used",                !Utility.bit(eFuse, 3)),
                new ParmDialog.ParmItem("*Not Used",                !Utility.bit(eFuse, 2)),
                new ParmDialog.ParmItem("*Not Used",                !Utility.bit(eFuse, 1)),
                new ParmDialog.ParmItem("SELFPRGEN{*[SELFPRGEN]*}", !Utility.bit(eFuse, 0)),
            }
            };
            String[] tabs = {"LFUSE", "HFUSE", "EFUSE"};
            ParmDialog dialog = new ParmDialog(parmSet, tabs, new String[] {"Program", "Cancel"});
            dialog.setLocationRelativeTo(this);
            dialog.setVisible(true);              // Note: this call invokes dialog
            if (dialog.wasPressed()) {
              // Read current fuse settings
              Map<String, String> tags = new HashMap<>();
              tags.put("PROG", icspProgrammer);
              tags.put("TDIR", tmpDir);
              tags.put("CHIP", chip);
              String exec = Utility.replaceTags("avrdude -Pusb -c *[PROG]* -p *[CHIP]* -U lfuse:r:*[TDIR]*lfuse.hex:h " +
                "-U hfuse:r:*[TDIR]*hfuse.hex:h -U efuse:r:*[TDIR]*efuse.hex:h", tags);
              String cmd = tmpExe + "bin" + System.getProperty("file.separator") + exec;
              System.out.println("Run: " + cmd);
              Process proc = Runtime.getRuntime().exec(cmd);
              String ret = Utility.runCmd(proc);
              int retVal = proc.waitFor();
              if (retVal != 0) {
                progPane.append("Error: " + ret + "\n");
                return;
              }
              int[] chipFuses = {
                Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*lfuse.hex", tags)).trim()),
                Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*hfuse.hex", tags)).trim()),
                Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*efuse.hex", tags)).trim())
              };
              StringBuilder out = new StringBuilder();
              for (int ii = 0; ii < chipFuses.length; ii++) {
                int dFuse = dialog.fuseSet[ii].fuseValue;
                int cFuse = chipFuses[ii];
                tags.put("FUSE", tabs[ii].toLowerCase());
                tags.put("FVAL", "0x" + Integer.toHexString(dFuse).toUpperCase());
                if (dFuse != cFuse) {
                  // Update fuse value
                  exec = Utility.replaceTags("avrdude -Pusb -c *[PROG]* -p *[CHIP]* -U *[FUSE]*:w:*[FVAL]*:m", tags);
                  cmd = tmpExe + "bin" + System.getProperty("file.separator") + exec;
                  System.out.println("Run: " + cmd);
                  proc = Runtime.getRuntime().exec(cmd);
                  ret = Utility.runCmd(proc);
                  retVal = proc.waitFor();
                  if (retVal != 0) {
                    progPane.append("Error: " + ret + "\n");
                    return;
                  } else {
                    out.append(ret);
                  }
                  System.out.println(tabs[ii] + " from 0x" + Integer.toHexString(cFuse).toUpperCase() +
                                    " to 0x" + Integer.toHexString(dFuse).toUpperCase());
                } else {
                  out.append("Leaving ").append(tags.get("FUSE")).append(" unchanged at ").append(tags.get("FVAL")).append("\n");
                }
                progPane.setText(out.toString());
              }
            } else {
              System.out.println("Cancel");
            }
          }
        }
      } catch (Exception ex) {
        ex.printStackTrace();
      }
    });
    actions.addSeparator();
    actions.add(mItem = new JMenuItem("Reinstall Toolchain"));
    mItem.addActionListener(e -> installToolchain(true));
    menuBar.add(actions);
    // Add Settings menu
    JMenu settings = new JMenu("Settings");
    menuBar.add(settings);
    JMenu tpiSettings = new JMenu("TPI Programmer");
    settings.add(tpiSettings);
    tpiSettings.setEnabled(false);
    Thread portThread = new Thread(() -> {
      // Add "Port" and "Baud" Menus to MenuBar
      try {
        jPort = new JSSCPort(prefs);
        tpiSettings.add(jPort.getPortMenu());
        tpiSettings.add(jPort.getBaudMenu());
        jPort.setRXHandler(this);
      } catch (Exception ex) {
        ex.printStackTrace();
      } finally {
        tpiSettings.setEnabled(true);
      }
    });
    portThread.start();
    JMenu icspProg = new JMenu("ICSP Programmer");
    try {
      Map<String,String> pgrmrs = Utility.toTreeMap(Utility.getResourceMap("icsp_programmers.props"));
      for (String key : pgrmrs.keySet()) {
        String val = pgrmrs.get(key);
        JRadioButtonMenuItem item = new JRadioButtonMenuItem(val);
        icspProg.add(item);
        if (icspProgrammer.equals(key)) {
          item.setSelected(true);
        }
        item.addActionListener(ex -> {
          prefs.put("icsp_programmer", icspProgrammer = key);
          System.out.println(key);
        });
      }
    } catch (Exception ex) {
      ex.printStackTrace();
    }
    settings.add(icspProg);
    setJMenuBar(menuBar);
    // Add window close handler
    addWindowListener(new WindowAdapter() {
      public void windowClosing (WindowEvent ev) {
        if (!codeDirty  ||  discardChanges()) {
          if (jPort != null && jPort.isOpen()) {
            jPort.close();
          }
          System.exit(0);
        }
      }
    });
    // Track window resize/move events and save in prefs
    addComponentListener(new ComponentAdapter() {
      public void componentMoved (ComponentEvent ev)  {
        Rectangle bounds = ev.getComponent().getBounds();
        prefs.putInt("window.x", bounds.x);
        prefs.putInt("window.y", bounds.y);
      }
      public void componentResized (ComponentEvent ev)  {
        Rectangle bounds = ev.getComponent().getBounds();
        prefs.putInt("window.width", bounds.width);
        prefs.putInt("window.height", bounds.height);
      }
    });
    setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
    setSize(prefs.getInt("window.width", 800), prefs.getInt("window.height", 900));
    setLocation(prefs.getInt("window.x", 10), prefs.getInt("window.y", 10));
    setVisible(true);
    installToolchain(prefs.getBoolean("reload_toolchain", false));
  }

  private boolean canProgram () {
    if (compiled || directHex) {
      return true;
    } else {
      showErrorDialog("Code not built!");
    }
    return false;
  }

  private void installToolchain (boolean reinstall) {
    // Create temporary scratch directory for compiler
    File tmp = (new File(System.getProperty("java.io.tmpdir") + "avr-temp-code"));
    if (!tmp.exists()) {
      tmp.mkdir();
    }
    try {
      tmpDir = tmp.getAbsolutePath() + System.getProperty("file.separator");
      // Install AVR Toolchain if it's not already installed
      tmp = (new File(System.getProperty("java.io.tmpdir") + "avr-toolchain"));
      if (reinstall || !tmp.exists()) {
        tmp.mkdir();
        tmpExe = tmp.getAbsolutePath() + System.getProperty("file.separator");
        if (osName.contains("windows")) {
          new ToolchainLoader(this, "WinToolchain.zip", tmpExe);
        } else if (osName.contains("mac")) {
          new ToolchainLoader(this, "MacToolchain.zip", tmpExe);
        } else {
          throw new IllegalStateException("Unsupported os: " + osName);
        }
      } else {
        tmpExe = tmp.getAbsolutePath() + System.getProperty("file.separator");
      }
      prefs.remove("reload_toolchain");
    } catch (Exception ex) {
      ex.printStackTrace();
    }
  }

  static class ProgressBar extends JFrame {
    private JDialog       frame;
    private JProgressBar  progress;
    private JTextArea     txt;

    ProgressBar (JFrame comp, String msg) {
      frame = new JDialog(comp);
      frame.setUndecorated(true);
      JPanel pnl = new JPanel(new BorderLayout());
      pnl.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
      frame.add(pnl, BorderLayout.CENTER);
      pnl.add(progress = new JProgressBar(), BorderLayout.NORTH);
      txt = new JTextArea(msg);
      txt.setEditable(false);
      pnl.add(txt, BorderLayout.SOUTH);
      Rectangle loc = comp.getBounds();
      frame.pack();
      frame.setLocation(loc.x + loc.width / 2 - 150, loc.y + loc.height / 2 - 150);
      frame.setVisible(true);
    }

    void setMessage (String msg) {
      txt.setText(msg);
    }
    void setValue (int value) {
      progress.setValue(value);
    }

    void setMaximum (int value) {
      progress.setMaximum(value);
    }

    void close () {
      frame.setVisible(false);
      frame.dispose();
    }
  }

  class ToolchainLoader extends ProgressBar implements Runnable  {
    private String        srcZip, tmpExe;

    ToolchainLoader (JFrame comp, String srcZip, String tmpExe) {
      super(comp, "Loading AVR Toolchain");
      this.srcZip = srcZip;
      this.tmpExe = tmpExe;
      (new Thread(this)).start();
    }

    public void run () {
      try {
        File dst = new File(tmpExe);
        if (!dst.exists()) {
          dst.mkdir();
        }
        infoPane.append("srcZip: " + srcZip + "\n");
        ZipFile zip = null;
        try {
          Path file = Files.createTempFile(null, ".zip");
          InputStream stream = this.getClass().getResourceAsStream(srcZip);
          Files.copy(stream, file, StandardCopyOption.REPLACE_EXISTING);
          File srcFile = file.toFile();
          srcFile.deleteOnExit();
          zip = new ZipFile(srcFile);
          int entryCount = 0, lastEntryCount = 0;
          setMaximum(zip.size());
          Enumeration entries = zip.entries();
          while (entries.hasMoreElements()) {
            ZipEntry entry = (ZipEntry) entries.nextElement();
            entryCount++;
            if (entryCount - lastEntryCount > 100) {
              setValue(lastEntryCount = entryCount);
            }
            String src = entry.getName();
            if (src.contains("MACOSX")) {
              continue;
            }
            File dstFile = new File(dst, src);
            File dstDir = dstFile.getParentFile();
            if (!dstDir.exists()) {
              dstDir.mkdir();
            }
            if (entry.isDirectory()) {
              dstFile.mkdirs();
            } else {
              try (ReadableByteChannel srcChan = Channels.newChannel(zip.getInputStream(entry));
                   FileChannel dstChan = new FileOutputStream(dstFile).getChannel()) {
                dstChan.transferFrom(srcChan, 0, entry.getSize());
              }
              // Must set permissions after file is written or it doesn't take...
              if (!dstFile.getName().contains(".")) {
                dstFile.setExecutable(true);
              }
            }
          }
        } finally {
          if (zip != null) {
            zip.close();
          }
        }
      } catch (Exception ex) {
        ex.printStackTrace();
      }
      close();
    }
  }

  // Implement JSSCPort.RXEvent
  public void rxChar (byte cc) {
    if (progPane != null) {
      progPane.append(Character.toString((char) cc));
    }
  }

  private JTextArea getScrollingTextPage (JTabbedPane tabs, String tabName, String hoverText) {
    JTextArea ta = new JTextArea();
    ta.setFont(tFont);
    ta.setTabSize(4);
    JScrollPane scroll = new JScrollPane(ta);
    tabs.addTab(tabName, null, scroll, hoverText);
    ta.setEditable(false);
    return ta;
  }

  static class CodeImage {
    private byte[]  data;
    private byte    fuses;

    CodeImage (byte[] data, byte fuses) {
      this.data = data;
      this.fuses = fuses;
    }
  }

  private static CodeImage parseIntelHex (String hex) {
    byte fuses = 0x0F;
    ArrayList<Byte> buf = new ArrayList<>();
    nextLine:
    for (String line : hex.split("\\s")) {
      if (line.startsWith(":")  && line.length() > 11) {
        int state = 0, count = 0, add = 0, chk = 0;
        for (int ii = 1; ii < line.length() - 1; ii += 2) {
          int msn = Utility.fromHex(line.charAt(ii));
          int lsn = Utility.fromHex(line.charAt(ii + 1));
          int val = (msn << 4) + lsn;
          switch (state) {
          case 0:
            count = val;
            chk += val;
            state++;
            break;
          case 1:
          case 2:
            // Collect 2 bytes into a 16 bit address
            add = (add << 8) + val;
            if (add + count > buf.size()) {
              buf.ensureCapacity(add + count);
            }
            chk += val;
            state++;
            break;
          case 3:
            // Check if this is a data record
            chk += val;
            if (val != 0) {
              continue nextLine;
            }
            state++;
            break;
          case 4:
            // Read data bytes
            if (count > 0) {
              buf.add(add++, (byte) val);
              chk += val;
              count--;
            }
            if (count == 0) {
              state++;
            }
            break;
          case 5:
            // Verify checksum
            chk += val;
            if ((chk & 0xFF) != 0) {
              throw new IllegalStateException("Invalid checksum in HEX file");
            }
            continue nextLine;
          }
        }
      } else if (line.startsWith("*")  && line.length() == 2) {
        fuses = (byte) Utility.fromHex(line.charAt(1));
      }
    }
    byte[] code = new byte[buf.size()];
    for (int ii = 0; ii < code.length; ii++) {
      code[ii] = buf.get(ii);
    }
    return new CodeImage(code, fuses);
  }

  private boolean discardChanges () {
    return doWarningDialog("Discard Changes?");
  }

  private void showErrorDialog (String msg) {
    ImageIcon icon = new ImageIcon(Utility.class.getResource("images/warning-32x32.png"));
    showMessageDialog(this, msg, "Error", JOptionPane.PLAIN_MESSAGE, icon);
  }

  private boolean doWarningDialog (String question) {
    ImageIcon icon = new ImageIcon(Utility.class.getResource("images/warning-32x32.png"));
    return JOptionPane.showConfirmDialog(this, question, "Warning", JOptionPane.YES_NO_OPTION,
      JOptionPane.WARNING_MESSAGE, icon) == JOptionPane.OK_OPTION;
  }

  public static void main (String[] args) {
    java.awt.EventQueue.invokeLater(ATTinyC::new);
  }
}
