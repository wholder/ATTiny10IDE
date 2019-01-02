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
import java.util.List;
import java.util.prefs.Preferences;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import javax.swing.*;
import javax.swing.event.HyperlinkEvent;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.text.Document;

import jssc.SerialNativeInterface;

import static javax.swing.JOptionPane.INFORMATION_MESSAGE;
import static javax.swing.JOptionPane.showMessageDialog;

/**
   *  An IDE for ATTiny10 Series Microcontrollers
   *  Author: Wayne Holder, 2017
   *  License: MIT (https://opensource.org/licenses/MIT)
   */

public class ATTinyC extends JFrame implements JSSCPort.RXEvent {
  private static final String       VERSION = "1.0 beta";
  private static final String       fileSep =  System.getProperty("file.separator");
  private static String             tempBase = System.getProperty("java.io.tmpdir");
  private static Font               tFont = getCodeFont(12);
  private static int                cmdMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
  private static KeyStroke          OPEN_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_O, cmdMask) ;
  private static KeyStroke          SAVE_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_S, cmdMask) ;
  private static KeyStroke          QUIT_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_Q, cmdMask) ;
  private static KeyStroke          BUILD_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_B, cmdMask) ;
  static Map<String,ChipInfo>       progProtocol = new HashMap<>();
  private enum                      Tab {DOC(0), SRC(1), LIST(2), HEX(3), PROG(4), INFO(5); final int num; Tab(int num) {this.num = num;}}
  private String                    osName = System.getProperty("os.name").toLowerCase();
  private enum                      OpSys {MAC, WIN, LINUX}
  private OpSys                     os;
  private JTabbedPane               tabPane;
  private JFileChooser              fc = new JFileChooser();
  private CodeEditPane              codePane;
  private MyTextPane                listPane, hexPane, progPane, infoPane;
  private JMenuItem                 saveMenu;
  private String                    tmpDir, tmpExe, chip, editFile;
  private boolean                   directHex, compiled, codeDirty;
  private File                      cFile;
  private transient Preferences     prefs = Preferences.userRoot().node(this.getClass().getName());
  private String                    ispProgrammer = prefs.get("icsp_programmer", "stk500v2");
  private transient JSSCPort        jPort;
  private Map<String, String>       compileMap;
  private static Map<String,String> sigLookup = new HashMap<>();

  {
    try {
      tempBase = tempBase.endsWith(fileSep) ? tempBase : tempBase + fileSep;
      if (osName.contains("win")) {
        os = OpSys.WIN;
        UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
      } else if (osName.contains("mac")) {
        os = OpSys.MAC;
      } else if (osName.contains("linux")) {
        os = OpSys.LINUX;
        UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
      } else {
        showErrorDialog("Unsupported os: " + osName);
        System.exit(1);
      }
    } catch (Exception ex) {
      ex.printStackTrace();
    }
  }

  static class ChipInfo {
    String  prog, lib, fuses, signature;
    boolean useCore;

    ChipInfo (String prog, String lib, String fuses, String signature, boolean useCore) {
      this.prog = prog;
      this.lib = lib;
      this.fuses = fuses;
      this.signature = signature;
      this.useCore = useCore;
    }
  }

  static class MyTextPane extends JEditorPane {
    MyTextPane (JTabbedPane tabs, String tabName, String hoverText) {
      setContentType("text/plain");
      setFont(tFont);
      //ta.setTabSize(4);
      JScrollPane scroll = new JScrollPane(this);
      tabs.addTab(tabName, null, scroll, hoverText);
      setEditable(false);
    }

    @Override
    public void setText (String text) {
      setContentType("text/plain");
      setFont(tFont);
      super.setText(text);
    }

    void setErrorText (String text) {
      setContentType("text/html");
      setFont(tFont);
      super.setText(text);
    }

    void append (String text) {
      Document doc = getDocument();
      try {
        doc.insertString(doc.getLength(), text, null);
        repaint();
      } catch (Exception ex) {
        ex.printStackTrace();
      }
    }
  }

  private static void addChip(String name, ChipInfo info) {
    progProtocol.put(name, info);
    sigLookup.put(info.signature, name);
  }

  static {
    // List of ATtiny types, Protocol used to Program them, etc.
    //       Part Name             Protocol   Library   Fuse(s)         Signature  UseCore
    addChip("attiny4",  new ChipInfo("TPI",  "tiny10", "FF",             "1E8F0A", false));
    addChip("attiny5",  new ChipInfo("TPI",  "tiny10", "FF",             "1E8F09", false));
    addChip("attiny9",  new ChipInfo("TPI",  "tiny10", "FF",             "1E9008", false));
    addChip("attiny10", new ChipInfo("TPI",  "tiny10", "FF",             "1E9003", false));
    addChip("attiny24", new ChipInfo("ISP",  "tinyX4", "l:60,h:DF,e:FF", "1E910B", true));
    addChip("attiny44", new ChipInfo("ISP",  "tinyX4", "l:60,h:DF,e:FF", "1E9207", true));
    addChip("attiny84", new ChipInfo("ISP",  "tinyX4", "l:60,h:DF,e:FF", "1E930C", true));
    addChip("attiny25", new ChipInfo("ISP",  "tinyX5", "l:60,h:DF,e:FF", "1E9108", true));
    addChip("attiny45", new ChipInfo("ISP",  "tinyX5", "l:60,h:DF,e:FF", "1E9206", true));
    addChip("attiny85", new ChipInfo("ISP",  "tinyX5", "l:60,h:DF,e:FF", "1E930B", true));
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
    prefs.putBoolean("developer_features", prefs.getBoolean("developer_features", false));
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
        "java.io.tmpdir: " + tempBase + "\n" +
        "tmpDir: " + tmpDir + "\n" +
        "tmpExe: " + tmpExe + "\n" +
        "Java Version: " + System.getProperty("java.version") + "\n" +
        "Java Simple Serial Connector: " + SerialNativeInterface.getLibraryVersion() + "\n" +
        "JSSC Native Code DLL Version: " + SerialNativeInterface.getNativeLibraryVersion() + "\n",
        "ATtiny10IDE " + VERSION, INFORMATION_MESSAGE,  icon);
  }

  private void showPreferences (int modifiers) {
    List<ParmDialog.ParmItem> items = new ArrayList<>();
    items.add(new ParmDialog.ParmItem("Generate Prototypes (Experimental){*[GEN_PROTOS]*}",
                                      prefs.getBoolean("gen_prototypes", true)));
    boolean devFeatures = (modifiers & InputEvent.CTRL_MASK) != 0;
    if (devFeatures) {
      items.add(new ParmDialog.ParmItem("Enable Preprocessing (Developer){*[PREPROCESS]*}",
                                        prefs.getBoolean("enable_preprocessing", false)));
      items.add(new ParmDialog.ParmItem("Enable Developer Features{*[DEV_ONLY]*}",
                                        prefs.getBoolean("developer_features", false)));
    }
    ParmDialog.ParmItem[] parmSet = items.toArray(new ParmDialog.ParmItem[0]);
    ParmDialog dialog = new ParmDialog(new ParmDialog.ParmItem[][] {parmSet}, null, new String[] {"Save", "Cancel"});
    dialog.setLocationRelativeTo(this);
    dialog.setVisible(true);              // Note: this call invokes dialog
    if (dialog.wasPressed()) {
      prefs.putBoolean("gen_prototypes",        parmSet[0].value);
      if (devFeatures) {
        prefs.putBoolean("enable_preprocessing",  parmSet[1].value);
        prefs.putBoolean("developer_features",    parmSet[2].value);
      }
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
    MarkupView howToPane = new MarkupView("documentation/index.md");
    tabPane.addTab("How To", null, howToPane, "This is the documentation page");
    tabPane.addTab("Source Code", null, codePane, "This is the editor pane where you enter source code");
    listPane = new MyTextPane(tabPane, "Listing", "Select this pane to view the assembler listing");
    listPane.addHyperlinkListener(ev -> {
      if (ev.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
        String [] tmp = ev.getDescription().split(":");
        tabPane.setSelectedIndex(0);
        int line = Integer.parseInt(tmp[0]);
        int column = Integer.parseInt(tmp[1]);
        codePane.setPosition(line, column);
        System.out.println(ev.getDescription());
      }
    });
    hexPane =  new MyTextPane(tabPane, "Hex Output", "Intel Hex Output file for programmer");
    progPane = new MyTextPane(tabPane, "Programmer", "Records communication with Arduino-based programmer");
    infoPane = new MyTextPane(tabPane, "Error Info", "Displays additional information about errors");
    infoPane.append("os.name: " + osName + "\n");
    infoPane.append("os:      " + os.toString() + "\n");
    // Add menu bar and menus
    JMenuBar menuBar = new JMenuBar();
    // Add "File" Menu
    JMenu fileMenu = new JMenu("File");
    JMenuItem mItem;
    fileMenu.add(mItem = new JMenuItem("About"));
    mItem.addActionListener(e -> showAboutBox());
    fileMenu.add(mItem = new JMenuItem("Preferences"));
    mItem.addActionListener(e -> showPreferences(e.getModifiers()));
    fileMenu.addSeparator();
    fileMenu.add(mItem = new JMenuItem("New"));
    mItem.addActionListener(e -> {
      if (codePane.getText().length() == 0 || discardChanges()) {
        codePane.setForeground(Color.black);
        codePane.setCode("");
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
            String src = Utility.getFile(oFile);
            if (src.contains(":00000001FF")) {
              if (!src.startsWith(":020000020000FC")) {
                src = ":020000020000FC\n" + src;
              }
              hexPane.setForeground(Color.black);
              hexPane.setText(src);
              selectTab(Tab.HEX);
              directHex = true;
            } else {
              cFile = oFile;
              codePane.setForeground(Color.black);
              String[] tmp = Utility.decodeMarkdown(src);
              codePane.setCode(tmp[0]);
              if (tmp.length > 1) {
                codePane.setMarkup(tmp[1]);
              }
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
    fileMenu.addSeparator();
    fileMenu.add(mItem = new JMenuItem("Quit ATTiny10IDE"));
    mItem.setAccelerator(QUIT_KEY);
    mItem.addActionListener(e -> {
      if (!codeDirty  ||  discardChanges()) {
        if (jPort.isOpen()) {
          jPort.close();
        }
        System.exit(0);
      }
    });
    menuBar.add(fileMenu);
    // Add "Edit" Menu
    menuBar.add(codePane.getEditMenu());
    // Add "Actions" Menu
    JMenu actions = new JMenu("Actions");
    actions.add(mItem = new JMenuItem("Build"));
    mItem.setToolTipText("Complile Code in Source Code Pane and Display Result in Listing and Hex Output Panes");
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
              String compName = "Sketch.cpp";
              String trueName = cFile.getName();
              if (compileMap.containsKey("ERR")) {
                listPane.setForeground(Color.red);
                // Remove path to tmpDir from error messages
                String errText = compileMap.get("ERR").replace(tmpDir + compName, trueName);
                errText = errText.replace("\n", "<br>");
                Pattern lineRef = Pattern.compile("(" + trueName + ":([0-9]+?:[0-9]+?):) error:");
                Matcher mat = lineRef.matcher(errText);
                StringBuffer buf = new StringBuffer("<html><body><tt>");
                while (mat.find()) {
                  String seq = mat.group(1);
                  if (seq != null) {
                    mat.appendReplacement(buf, "<a href=\"" + mat.group(2) +  "\">" + seq + "</a>");
                  }
                }
                mat.appendTail(buf);
                buf.append("</tt></body></html>");
                listPane.setErrorText(buf.toString());
                compiled = false;
              } else {
                listPane.setForeground(Color.black);
                String listing = compileMap.get("INFO") + "\n\n" + compileMap.get("SIZE") + compileMap.get("LST");
                compName = compName.substring(0, compName.indexOf("."));
                trueName = trueName.substring(0, trueName.indexOf("."));
                listPane.setText(listing.replace(tmpDir + compName, trueName));
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
    preprocess.setToolTipText("Run GCC Propressor and Display Result in Listing Pane");
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
    actions.addSeparator();
    /*
     *    TPI Programmer Menus
     */
    JMenu tpiProg = new JMenu("TPI Programmer");
    actions.add(tpiProg);
    actions.addSeparator();
    tpiProg.add(mItem = new JMenuItem("Program Device"));
    mItem.setToolTipText("Commands TPI Programmer to Upload and Program Code to Device");
    mItem.addActionListener(e -> {
      try {
        if (canProgram()) {
          String hex = hexPane.getText();
          String protocol = progProtocol.get(chip.toLowerCase()).prog;
          if ("TPI".equals(protocol)) {
            if (jPort.isOpen()  ||  jPort.reopen()) {
              selectTab(Tab.PROG);
              progPane.append("\nSending Code for: " + cFile.getName());
              jPort.sendString("\nD\n" + hex + "\n");
            } else {
              showErrorDialog("Serial port not selected!");
            }
          } else {
            showErrorDialog("TPI Programming is not complatible with selected device");
          }
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Generate Arduino Programmer Code"));
    mItem.setToolTipText("Generates Arduino Sketch that can Program Device with Built Code");
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
        case "ISP":
          showErrorDialog("Protocol '" + protocol + " not currently supported");
          break;
        }
      }
    });
    tpiProg.add(mItem = new JMenuItem("Calibrate Clock"));
    mItem.setToolTipText("Commands TPI Programmer to Upload and Run Clock Calibration Code on Device");
    mItem.addActionListener(e -> {
      try {
        if (jPort.isOpen()  ||  jPort.reopen()) {
          selectTab(Tab.PROG);
          String hex = Utility.getFile("res:clockcal.hex");
          progPane.append("\nProgramming Clock Code");
          jPort.sendString("\nD\n" + hex + "\n");
          jPort.sendString("M\n");
        } else {
          showErrorDialog("Serial Port not selected!");
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Device Signature"));
    mItem.setToolTipText("Commands TPI Programmer to Read and Send Back Device's Signature");
    mItem.addActionListener(e -> {
      try {
        if (jPort.isOpen()  ||  jPort.reopen()) {
          selectTab(Tab.PROG);
          jPort.sendString("S\n");
        } else {
          showErrorDialog("Serial Port not selected!");
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Power On"));
    mItem.setToolTipText("Commands TPI Programmer to Switch On Power to Device");
    mItem.addActionListener(e -> {
      try {
        if (jPort.isOpen()  ||  jPort.reopen()) {
          jPort.sendString("V\n");
        } else {
          showErrorDialog("Serial Port not selected!");
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tpiProg.add(mItem = new JMenuItem("Power Off"));
    mItem.setToolTipText("Commands TPI Programmer to Switch Off Power to Device");
    mItem.addActionListener(e -> {
      try {
        if (jPort.isOpen()  ||  jPort.reopen()) {
          jPort.sendString("X\n");
        } else {
          showErrorDialog("Serial Port not selected!");
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    /*
     *    ISP Programmer Menus
     */
    JMenu icpProg = new JMenu("ISP Programmer");
    actions.add(icpProg);
    icpProg.add(mItem = new JMenuItem("Program Device"));
    mItem.setToolTipText("Commands Selected ISP Programmer Upload and Program Code to Device");
    mItem.addActionListener(e -> {
      if (chip == null) {
        showErrorDialog("Chip type not defined (code must be built to define)");
        return;
      }
      selectTab(Tab.PROG);
      progPane.setText("");
      Thread tt = new Thread(() -> {
      try {
        if (canProgram()) {
          String hex = hexPane.getText();
          String protocol = progProtocol.get(chip.toLowerCase()).prog;
          if (protocol.equals("ISP")) {
            // Copy contents of "hex" pane to temp file with .hex extension
            FileOutputStream fOut = new FileOutputStream(tmpDir + "code.hex");
            fOut.write(hex.getBytes(StandardCharsets.UTF_8));
            fOut.close();
            // Use AVRDUDE to program chip
            Map<String, String> tags = new HashMap<>();
            int retVal = callAvrdude("-U flash:w:*[TDIR]*code.hex", tags);
            if (retVal != 0) {
              showErrorDialog("Error Programming Device Signature with " + ispProgrammer);
            }
          } else {
            showErrorDialog("ISP Programming is not complatible with selected device");
          }
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    tt.start();
    });
    icpProg.add(mItem = new JMenuItem("Device Signature"));
    mItem.setToolTipText("Commands Selected ISP Programmer to Read and Send Back Device's Signature");
    mItem.addActionListener(e -> {
      selectTab(Tab.PROG);
      progPane.setText("");
      Thread tt = new Thread(() -> {
      try {
        Map<String, String> tags = new HashMap<>();
        int retVal = callAvrdude("-U signature:r:*[TDIR]*sig.hex:h", tags);
        if (retVal != 0) {
          showErrorDialog("Error Reading Device Signature with " + ispProgrammer);
          return;
        }
        String tmp = Utility.getFile(Utility.replaceTags("*[TDIR]*sig.hex", tags)).trim().toUpperCase().replace('X', 'x');
        String[] sigBytes = tmp.split(",");
        StringBuilder buf = new StringBuilder();
        for (String sigByte : sigBytes) {
          String val = sigByte.split("x")[1];
          buf.append(val.length() == 2 ? val : "0" + val);
        }
        String device = sigLookup.get(buf.toString());
        progPane.append("Device Signature: " + tmp + " - " + (device != null ? device : "unknown") + "\n");
      } catch (Exception ex) {
        ex.printStackTrace();
      }
      });
      tt.start();
    });

    icpProg.add(mItem = new JMenuItem("Read Fuses"));
    mItem.setToolTipText("Commands Selected ISP Programmer to Read and Send Back Device's Fuse Settings");
    mItem.addActionListener(e -> {
      if (chip == null) {
        showErrorDialog("Chip type not defined (code must be built to define)");
        return;
      }
      selectTab(Tab.PROG);
      progPane.setText("");
      ATTinyC ref = this;
      Thread tt = new Thread(() -> {
        try {
          // Read current fuse settings from chip
          Map<String, String> tags = new HashMap<>();
          int retVal = callAvrdude("-U lfuse:r:*[TDIR]*lfuse.hex:h -U hfuse:r:*[TDIR]*hfuse.hex:h -U efuse:r:*[TDIR]*efuse.hex:h", tags);
          if (retVal != 0) {
            showErrorDialog("Error Reading Fuses with " + ispProgrammer);
            return;
          }
          int lFuse = Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*lfuse.hex", tags)).trim());
          int hFuse = Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*hfuse.hex", tags)).trim());
          int eFuse = Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*efuse.hex", tags)).trim());
          ParmDialog.ParmItem[][] parmSet = getFuseParms(lFuse, hFuse, eFuse);
          for (ParmDialog.ParmItem[] col : parmSet) {
            for (ParmDialog.ParmItem parm : col) {
              parm.readOnly = true;
            }
          }
          String[] tabs = {"LFUSE", "HFUSE", "EFUSE"};
          ParmDialog dialog = new ParmDialog(parmSet, tabs, new String[] {"OK"});
          dialog.setLocationRelativeTo(ref);
          dialog.setVisible(true);              // Note: this call invokes dialog
        } catch (Exception ex) {
          ex.printStackTrace();
        }
      });
      tt.start();
    });
    icpProg.add(mItem = new JMenuItem("Program Fuses"));
    mItem.setToolTipText("Commands Selected ISP Programmer to Program Device's Fuses to New Settings");
    mItem.addActionListener(e -> {
      if (chip == null) {
        showErrorDialog("Chip type not defined (code must be built to define)");
        return;
      }
      selectTab(Tab.PROG);
      progPane.setText("");
      Thread tt = new Thread(() -> {
        try {
        if (canProgram()) {
          ChipInfo chipInfo = progProtocol.get(chip.toLowerCase());
          progPane.setText("");
          Map<String,String> fuseDefaults = Utility.csvToMap(chipInfo.fuses);
          int lFuse = Integer.decode(Utility.choose(compileMap.get("PRAGMA.LFUSE"), "0x" + fuseDefaults.get("l")));
          int hFuse = Integer.decode(Utility.choose(compileMap.get("PRAGMA.HFUSE"), "0x" + fuseDefaults.get("h")));
          int eFuse = Integer.decode(Utility.choose(compileMap.get("PRAGMA.EFUSE"), "0x" + fuseDefaults.get("e")));
          ParmDialog.ParmItem[][] parmSet = getFuseParms(lFuse, hFuse, eFuse);
          String[] tabs = {"LFUSE", "HFUSE", "EFUSE"};
          ParmDialog dialog = new ParmDialog(parmSet, tabs, new String[] {"Program", "Cancel"});
          dialog.setLocationRelativeTo(this);
          dialog.setVisible(true);              // Note: this call invokes dialog
          if (dialog.wasPressed()) {
            // Read current fuse settings
            Map<String, String> tags = new HashMap<>();
            int retVal = callAvrdude("-U lfuse:r:*[TDIR]*lfuse.hex:h -U hfuse:r:*[TDIR]*hfuse.hex:h -U efuse:r:*[TDIR]*efuse.hex:h", tags);
            if (retVal != 0) {
              showErrorDialog("Error Reading Fuses with " + ispProgrammer);
              return;
            }
            int[] chipFuses = {
              Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*lfuse.hex", tags)).trim()),
              Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*hfuse.hex", tags)).trim()),
              Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*efuse.hex", tags)).trim())
            };
            for (int ii = 0; ii < chipFuses.length; ii++) {
              int dFuse = dialog.fuseSet[ii].fuseValue;
              int cFuse = chipFuses[ii];
              if (dFuse != cFuse) {
                // Update fuse value
                tags.put("FUSE", tabs[ii].toLowerCase());
                tags.put("FVAL", "0x" + Integer.toHexString(dFuse).toUpperCase());
                retVal = callAvrdude("-U *[FUSE]*:w:*[FVAL]*:m", tags);
                if (retVal != 0) {
                  showErrorDialog("Error Writing Fuses with " + ispProgrammer);
                  return;
                }
              } else {
                progPane.append("Fuse " + tabs[ii].toLowerCase() + " already set correctly, so left unchanged\n");
              }
            }
          }
        }
      } catch (Exception ex) {
        ex.printStackTrace();
      }
      });
      tt.start();
    });
    actions.addSeparator();
    actions.add(mItem = new JMenuItem("Reinstall Toolchain"));
    mItem.setToolTipText("Copies AVR Toolchain into Java Temporary Disk Space where it can be Executed");
    mItem.addActionListener(e -> installToolchain(true));
    menuBar.add(actions);
    // Add Settings menu
    JMenu settings = new JMenu("Settings");
    menuBar.add(settings);
    // Add "Tabs" Menu with submenu
    settings.add(codePane.getTabSizeMenu());
    settings.addSeparator();
    // Add "Foze Size" Menu with submenu
    settings.add(codePane.getFontSizeMenu());
    settings.addSeparator();
    JMenu tpiSettings = new JMenu("Serial Port");
    settings.add(tpiSettings);
    settings.addSeparator();
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
    JMenu icspProg = new JMenu("ISP Programmer");
    icspProg.setToolTipText("Select ISP Programmer");
    try {
      ButtonGroup icspGroup = new ButtonGroup();
      Map<String,String> pgrmrs = Utility.toTreeMap(Utility.getResourceMap("icsp_programmers.props"));
      for (String key : pgrmrs.keySet()) {
        String val = pgrmrs.get(key);
        int idxs = val.indexOf("{");
        int idxe = val.indexOf("}");
        String toolTip = null;
        if (idxs >= 0 && idxe > idxs) {
          toolTip = val.substring(idxs + 1, idxe);
          val = val.substring(0, idxs) + val.substring(idxe + 1);
        }
        JRadioButtonMenuItem item = new JRadioButtonMenuItem(val);
        icspGroup.add(item);
        if (toolTip != null) {
          item.setToolTipText("<html>" + toolTip + "</html>");
        }
        icspProg.add(item);
        if (ispProgrammer.equals(key)) {
          item.setSelected(true);
        }
        item.addActionListener(ex -> {
          prefs.put("icsp_programmer", ispProgrammer = key);
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
          if (jPort.isOpen()) {
            jPort.close();
          }
          System.exit(0);
        }
      }
    });
    setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
    setSize(prefs.getInt("window.width", 800), prefs.getInt("window.height", 900));
    setLocation(prefs.getInt("window.x", 10), prefs.getInt("window.y", 10));
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
    setVisible(true);
    installToolchain(prefs.getBoolean("reload_toolchain", false));
  }

  private int callAvrdude (String op,  Map<String, String> tags) throws Exception {
    tags.put("VBS", prefs.getBoolean("developer_features", false) ? "-v" : "");
    tags.put("PROG", ispProgrammer);
    tags.put("TDIR", tmpDir);
    tags.put("CHIP", chip != null ? chip : "attiny85");
    tags.put("CFG", tmpExe + "etc" + fileSep + "avrdude.conf");
    boolean usesPort = "arduino".equals(ispProgrammer) || "buspirate".equals(ispProgrammer);
    tags.put("OUT", usesPort ? "-P " + jPort.getPortName() + " -b 19200" : "-Pusb");
    if (usesPort) {
      jPort.close();
    }
    String exec = Utility.replaceTags("avrdude *[VBS]* *[OUT]* -C *[CFG]* -c *[PROG]* -p *[CHIP]* " + op, tags);
    String cmd = tmpExe + "bin" + fileSep + exec;
    System.out.println("Run: " + cmd);
    Process proc = Runtime.getRuntime().exec(cmd);
    Utility.runCmd(proc, progPane);
    return proc.waitFor();
  }

  private ParmDialog.ParmItem[][] getFuseParms (int lFuse, int hFuse, int eFuse) {
    return new ParmDialog.ParmItem[][]{
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
  }

  private boolean canProgram () {
    if (compiled || directHex) {
      return true;
    } else {
      showErrorDialog("Code not built!");
    }
    return false;
  }

  static Font getCodeFont (int points) {
    String os = System.getProperty("os.name").toLowerCase();
    if (os.contains("win")) {
      return new Font("Consolas", Font.PLAIN, points);
    } else if (os.contains("mac")) {
      return new Font("Menlo", Font.PLAIN, points);
    } else if (os.contains("linux")) {
      return new Font("Courier", Font.PLAIN, points);
    } else {
      return new Font("Courier", Font.PLAIN, points);
    }
  }

  private void installToolchain (boolean reinstall) {
    // Create temporary scratch directory for compiler
    File tmp = (new File(tempBase + "avr-temp-code"));
    if (!tmp.exists()) {
      tmp.mkdirs();
    }
    try {
      tmpDir = tmp.getAbsolutePath() + fileSep;
      // Install AVR Toolchain if it's not already installed
      tmp = (new File(tempBase + "avr-toolchain"));
      if (reinstall || !tmp.exists()) {
        tmp.mkdirs();
        tmpExe = tmp.getAbsolutePath() + fileSep;
        if (os == OpSys.WIN) {
          new ToolchainLoader(this, "toolchains/WinToolchain.zip", tmpExe);
        } else if (os == OpSys.MAC) {
          new ToolchainLoader(this, "toolchains/MacToolchain.zip", tmpExe);
        } else if (os == OpSys.LINUX) {
          new ToolchainLoader(this, "toolchains/L64Toolchain.zip", tmpExe);
        }
      } else {
        tmpExe = tmp.getAbsolutePath() + fileSep;
      }
      prefs.remove("reload_toolchain");
    } catch (Exception ex) {
      ex.printStackTrace();
      selectTab(Tab.LIST);
      listPane.setText("Unable to Install Toolchain:\n" + ex.toString());
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
      super(comp, "Installing AVR Toolchain");
      this.srcZip = srcZip;
      this.tmpExe = tmpExe;
      (new Thread(this)).start();
    }

    public void run () {
      try {
        File dst = new File(tmpExe);
        if (!dst.exists()) {
          dst.mkdirs();
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
              dstDir.mkdirs();
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
