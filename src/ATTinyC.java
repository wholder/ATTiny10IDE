import java.awt.*;
import java.awt.event.*;

import java.io.*;
import java.net.URI;
import java.net.URL;
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
import javax.swing.filechooser.FileFilter;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.text.Document;

import jssc.SerialNativeInterface;

import static javax.swing.JOptionPane.INFORMATION_MESSAGE;
import static javax.swing.JOptionPane.showMessageDialog;

/**
   *  An IDE for ATTiny10 Series Microcontrollers
   *  Author: Wayne Holder, 2011-2019
   *  License: MIT (https://opensource.org/licenses/MIT)
   */

public class ATTinyC extends JFrame implements JSSCPort.RXEvent {
  private static final String       VERSION_URL = "https://raw.githubusercontent.com/wholder/ATTiny10IDE/master/resources/version.props";
  private static final String       DOWNLOAD = "https://github.com/wholder/ATTiny10IDE/blob/master/out/artifacts/ATTiny10IDE_jar/ATTiny10IDE.jar";
  private static final String       fileSep =  System.getProperty("file.separator");
  private static String             tempBase = System.getProperty("java.io.tmpdir");
  private static Font               tFont = getCodeFont(12);
  private static int                cmdMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
  private static KeyStroke          OPEN_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_O, cmdMask) ;
  private static KeyStroke          SAVE_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_S, cmdMask) ;
  private static KeyStroke          QUIT_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_Q, cmdMask) ;
  private static KeyStroke          BUILD_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_B, cmdMask) ;
  static Map<String,ChipInfo>       progProtocol = new LinkedHashMap<>();
  private enum                      Tab {DOC(0), SRC(1), LIST(2), HEX(3), PROG(4), INFO(5); final int num; Tab(int num) {this.num = num;}}
  private String                    osName = System.getProperty("os.name").toLowerCase();
  private enum                      OpSys {MAC, WIN, LINUX}
  private OpSys                     os;
  private JTabbedPane               tabPane;
  private JFileChooser              fc = new JFileChooser();
  private CodeEditPane              codePane;
  private MyTextPane                listPane, hexPane, progPane, infoPane;
  private JMenuItem                 saveMenu;
  private RadioMenu                 targetMenu;
  private String                    tmpDir, tmpExe, avrChip, editFile, exportParms;
  private boolean                   directHex, compiled, codeDirty;
  private File                      cFile;
  private transient Preferences     prefs = Preferences.userRoot().node(this.getClass().getName());
  private Map<String, String>       compileMap;
  private Properties                versionInfo;
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
    String  prog, core, variant, libs, part, fuses, signature;

    ChipInfo (String prog, String core, String variant, String libs, String part, String fuses, String signature) {
      this.prog = prog;
      this.core = core;
      this.variant = variant;
      this.libs = libs;
      this.part = part;             // For AVRDUDE -p switch
      this.fuses = fuses;
      this.signature = signature;
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

  static class RadioMenu extends JMenu {
    RadioMenu (String name) {
      super(name);
    }

    void setSelected (String name) {
      for (int ii = 0; ii < getItemCount(); ii++) {
        JMenuItem item = getItem(ii);
        if (item != null && name.equals(item.getText())) {
          item.setSelected(true);
          return;
        }
      }
    }

    String getSelected () {
      for (int ii = 0; ii < getItemCount(); ii++) {
        JMenuItem item = getItem(ii);
        if (item != null && item.isSelected()) {
          return item.getText();
        }
      }
      return null;
    }
  }

  private static void addChip (String name, ChipInfo info) {
    progProtocol.put(name, info);
    sigLookup.put(info.signature, name);
  }

  static {
    // List of ATTiny types, Protocol used to Program them, etc.
    //       Part Name              Protocol  Core        Variant      Libs         Part  Fuse(s)          Signature
    addChip("attiny4",  new ChipInfo("TPI",  "core10",     null,      "lib10",    "t4",  "FF",             "1E8F0A"));
    addChip("attiny5",  new ChipInfo("TPI",  "core10",     null,      "lib10",    "t5",  "FF",             "1E8F09"));
    addChip("attiny9",  new ChipInfo("TPI",  "core10",     null,      "lib10",    "t9",  "FF",             "1E9008"));
    addChip("attiny10", new ChipInfo("TPI",  "core10",     null,      "lib10",    "t10", "FF",             "1E9003"));
    addChip("attiny24", new ChipInfo("ISP",  "coretiny",  "corex4",   "libtiny",  "t24", "l:60,h:DF,e:FF", "1E910B"));
    addChip("attiny44", new ChipInfo("ISP",  "coretiny",  "corex4",   "libtiny",  "t44", "l:60,h:DF,e:FF", "1E9207"));
    addChip("attiny84", new ChipInfo("ISP",  "coretiny",  "corex4",   "libtiny",  "t84", "l:60,h:DF,e:FF", "1E930C"));
    addChip("attiny25", new ChipInfo("ISP",  "coretiny",  "corex5",   "libtiny",  "t25", "l:60,h:DF,e:FF", "1E9108"));
    addChip("attiny45", new ChipInfo("ISP",  "coretiny",  "corex5",   "libtiny",  "t45", "l:60,h:DF,e:FF", "1E9206"));
    addChip("attiny85", new ChipInfo("ISP",  "coretiny",  "corex5",   "libtiny",  "t85", "l:60,h:DF,e:FF", "1E930B"));
  }

  {
    FileNameExtensionFilter[] filters = {
      new FileNameExtensionFilter("AVR .c , .cpp or .ino files", "c", "cpp", "ino"),
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
    String version = versionInfo.get("version") + " " + versionInfo.get("status");
    showMessageDialog(this,
      "By: Wayne Holder\n" +
        "java.io.tmpdir: " + tempBase + "\n" +
        "tmpDir: " + tmpDir + "\n" +
        "tmpExe: " + tmpExe + "\n" +
        "Java Version: " + System.getProperty("java.version") + "\n" +
        "Java Simple Serial Connector: " + SerialNativeInterface.getLibraryVersion() + "\n" +
        "JSSC Native Code DLL Version: " + SerialNativeInterface.getNativeLibraryVersion() + "\n",
        "ATtiny10IDE " + version, INFORMATION_MESSAGE,  icon);
  }

  private void showPreferences (int modifiers) {
    List<ParmDialog.ParmItem> items = new ArrayList<>();
    items.add(new ParmDialog.ParmItem("Generate Prototypes (Experimental){*[GEN_PROTOS]*}",
                                      prefs.getBoolean("gen_prototypes", true)));
    items.add(new ParmDialog.ParmItem("Interleave Source and ASM{*[INTERLEAVE]*}",
                                      prefs.getBoolean("interleave", true)));
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
      prefs.putBoolean("gen_prototypes",          parmSet[0].value);
      prefs.putBoolean("interleave",              parmSet[1].value);
      if (devFeatures) {
        prefs.putBoolean("enable_preprocessing",  parmSet[2].value);
        prefs.putBoolean("developer_features",    parmSet[3].value);
      }
    }
  }

  private ATTinyC () {
    super("ATTinyC");
    // Setup temp directory for code compilation
    File base = (new File(tempBase + "avr-temp-code"));
    if (!base.exists()) {
      base.mkdirs();
    }
    tmpDir = base.getAbsolutePath() + fileSep;
    // Setup temp directory for AVR Toolchain
    base = (new File(tempBase + "avr-toolchain"));
    if (!base.exists()) {
      base.mkdirs();
    }
    tmpExe = base.getAbsolutePath() + fileSep;
    // Load version info
    try {
      versionInfo = Utility.getResourceMap("version.props");
    } catch (IOException ex) {
      showErrorDialog("Unable to load version.props");
      ex.printStackTrace();
      System.exit(0);
    }
    // Setup interface
    setBackground(Color.white);
    setLayout(new BorderLayout(1, 1));
    // Create Tabbed Pane
    tabPane = new JTabbedPane();
    add("Center", tabPane);
    codePane = new CodeEditPane(prefs);
    codePane.setCodeChangeListener(() -> {
      setDirtyIndicator(codeDirty = true);
      updateChip(codePane.getText());
      compiled = false;
      listPane.setForeground(Color.red);
      hexPane.setForeground(Color.red);
    });
    MarkupView howToPane = new MarkupView("documentation/index.md");
    tabPane.addTab("How To", null, howToPane, "This is the documentation page");
    tabPane.addTab("Source Code", null, codePane, "This is the editor pane where you enter source code");
    listPane = new MyTextPane(tabPane, "Listing", "Select this pane to view the assembler listing");
    listPane.setEditable(false);
    listPane.addHyperlinkListener(ev -> {
      if (ev.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
        String [] tmp = ev.getDescription().split(":");
        selectTab(Tab.SRC);
        int line = Integer.parseInt(tmp[0]);
        int column = Integer.parseInt(tmp[1]);
        codePane.setPosition(line, column);
        //System.out.println(ev.getDescription());
      }
    });
    hexPane =  new MyTextPane(tabPane, "Hex Output", "Intel Hex Output file for programmer");
    progPane = new MyTextPane(tabPane, "Programmer", "Records communication with Arduino-based programmer");
    infoPane = new MyTextPane(tabPane, "Error Info", "Displays additional information about IDE and error messages");
    hexPane.setEditable(false);
    progPane.setEditable(false);
    infoPane.setEditable(false);
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
    // Add "Check for Updates" Menu item
    fileMenu.add(mItem = new JMenuItem("Check for Updates"));
    mItem.addActionListener(ev -> {
      // Check for new version available
      // https://github.com/wholder/ATTiny10IDE/blob/master/resources/version.props
      try {
        Properties latest = Utility.getResourceMap(new URL(VERSION_URL));
        String oldVersion = versionInfo.getProperty("version");
        String newVersion = latest.getProperty("version");
        if (oldVersion != null && newVersion != null) {
          try {
            float oldV = Float.parseFloat(oldVersion);
            float newV = Float.parseFloat(newVersion);
            if (newV > oldV) {
              String status = latest.getProperty("status");
              String version = newVersion + (status != null && status.length() > 0 ? " " + status : "");
              ImageIcon icon = new ImageIcon(Utility.class.getResource("images/info-32x32.png"));
              if (JOptionPane.showConfirmDialog(this, "<html>A new version (" + version + ") is available!<br>" +
                                     "Do you want to go to the download page?</html>", "Warning", JOptionPane.YES_NO_OPTION,
                  JOptionPane.WARNING_MESSAGE, icon) == JOptionPane.OK_OPTION) {
                if (Desktop.isDesktopSupported()) {
                  try {
                    Desktop.getDesktop().browse(new URI(DOWNLOAD));
                  } catch (Exception ex) {
                    ex.printStackTrace();
                  }
                }
              }
            } else {
              ImageIcon icon = new ImageIcon(Utility.class.getResource("images/info-32x32.png"));
              JOptionPane.showMessageDialog(this, "You have the latest version.", "Attention", INFORMATION_MESSAGE, icon);
            }
          } catch (NumberFormatException ex) {
            ex.printStackTrace();
          }
        }
      } catch (IOException ex) {
        ex.printStackTrace();
      }
    });
    fileMenu.addSeparator();
    fileMenu.add(mItem = new JMenuItem("New"));
    mItem.addActionListener(e -> {
      if (codePane.getText().length() == 0 || discardChanges()) {
        codePane.setForeground(Color.black);
        codePane.setCode("");
        directHex = false;
        compiled = false;
        editFile = null;
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
            FileFilter filter = fc.getFileFilter();
            if (filter instanceof FileNameExtensionFilter) {
              String[] exts = ((FileNameExtensionFilter) filter).getExtensions();
              prefs.put("default.extension",  exts[0]);
            }
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
        FileFilter filter = fc.getFileFilter();
        if (filter instanceof FileNameExtensionFilter) {
          String[] exts = ((FileNameExtensionFilter) filter).getExtensions();
          prefs.put("default.extension",  exts[0]);
        }
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
        System.exit(0);
      }
    });
    menuBar.add(fileMenu);
    // Add "Edit" Menu
    JMenu editMenu = codePane.getEditMenu();
    editMenu.setEnabled(false);
    menuBar.add(editMenu);
    tabPane.addChangeListener(ev -> {
      if (tabPane.getSelectedIndex() == Tab.SRC.num) {
        editMenu.setEnabled(true);
      } else {
        editMenu.setEnabled(false);
      }
    });
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
          verifyToolchain();
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
              compileMap = ATTinyCompiler.compile(codePane.getText(), tags, prefs, this);
              String compName = "Sketch.cpp";
              String trueName = cFile.getName();
              if (compileMap.containsKey("ERR")) {
                listPane.setForeground(Color.red);
                // Remove path to tmpDir from error messages
                String errText = compileMap.get("ERR").replace(tmpDir + compName, trueName);
                errText = errText.replace("\n", "<br>");
                Pattern lineRef = Pattern.compile("(" + trueName + ":([0-9]+?:[0-9]+?):) (fatal error|error|note):");
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
                StringBuilder tmp = new StringBuilder();
                tmp.append(compileMap.get("INFO"));
                tmp.append("\n\n");
                if (compileMap.containsKey("WARN")) {
                  tmp.append(compileMap.get("WARN"));
                  tmp.append("\n\n");
                }
                exportParms = compileMap.get("XPARMS");
                tmp.append( compileMap.get("SIZE"));
                tmp.append(compileMap.get("LST"));
                String listing = tmp.toString();
                compName = compName.substring(0, compName.indexOf("."));
                trueName = trueName.substring(0, trueName.indexOf("."));
                listPane.setText(listing.replace(tmpDir + compName, trueName));
                hexPane.setForeground(Color.black);
                hexPane.setText(compileMap.get("HEX"));
                avrChip = compileMap.get("CHIP");
                compiled = true;
              }
            } catch (Exception ex) {
              prefs.putBoolean("reload_toolchain", true);
              ex.printStackTrace();
              listPane.setText("Compile error (see Error Info pane for details)\n" + ex.toString());
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
          verifyToolchain();
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
              compileMap = ATTinyCompiler.compile(codePane.getText(), tags, prefs, this);
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
     *    Program Chip Menu
     */
    actions.add(mItem = new JMenuItem("Program Flash and Fuse(s)"));
    mItem.setToolTipText("Commands Programmer to Upload and Program Code and Fuses to Device");
    mItem.addActionListener(e -> {
      if (avrChip != null) {
        ChipInfo info = progProtocol.get(avrChip);
        if ("TPI".equals(info.prog) && isSerialProgrammer() && supportsTPI()) {
          // Program flash using ATTiny10GeneratedProgrammer sketch as TPI-based programmer
          try {
            if (canProgram()) {
              String hex = hexPane.getText();
              selectTab(Tab.PROG);
              progPane.setText("Sending Code for: " + cFile.getName() + "\n");
              sendToJPort("\nD\n" + hex + "\n");
            }
          } catch (Exception ex) {
            ex.printStackTrace();
            showErrorDialog(ex);
          }
        } else if ("TPI".equals(info.prog) && isAvrdudeProgrammer() && supportsTPI() ||
                   "ISP".equals(info.prog) && isAvrdudeProgrammer() && supportsISP()) {
          // Program flash using AVRDUDE jusing either TPI or ISP protocols
          selectTab(Tab.PROG);
          progPane.setText("");
          Thread tt = new Thread(() -> {
            try {
              if (canProgram()) {
                // If all fuses are defined in #pragma statements, verify and program, as needed
                if (compileMap.containsKey("PRAGMA.LFUSE") && compileMap.containsKey("PRAGMA.HFUSE") &&
                    compileMap.containsKey("PRAGMA.EFUSE")) {
                  // Read current fuse settings from chip
                  Map<String, String> tags = new HashMap<>();
                  int retVal = callAvrdude("-U lfuse:r:*[TDIR]*lfuse.hex:h -U hfuse:r:*[TDIR]*hfuse.hex:h -U efuse:r:*[TDIR]*efuse.hex:h", tags);
                  if (retVal != 0) {
                    showErrorDialog("Error Verifying Fuse setting with " + prefs.get("programmer.name", null));
                    return;
                  }
                  int[] pFuses = {
                      Integer.decode(compileMap.get("PRAGMA.LFUSE")),
                      Integer.decode(compileMap.get("PRAGMA.HFUSE")),
                      Integer.decode(compileMap.get("PRAGMA.EFUSE"))
                  };
                  int[] cFuses = {
                      Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*lfuse.hex", tags)).trim()),
                      Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*hfuse.hex", tags)).trim()),
                      Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*efuse.hex", tags)).trim())
                  };
                  String[] tabs = {"LFUSE", "HFUSE", "EFUSE"};
                  for (int ii = 0; ii < tabs.length; ii++) {
                    int pFuse = pFuses[ii];
                    int cFuse = cFuses[ii];
                    if (pFuse != cFuse) {
                      // Update fuse value
                      tags.put("FUSE", tabs[ii].toLowerCase());
                      tags.put("FVAL", "0x" + Integer.toHexString(pFuse).toUpperCase());
                      retVal = callAvrdude("-U *[FUSE]*:w:*[FVAL]*:m", tags);
                      if (retVal != 0) {
                        showErrorDialog("Error Writing Fuses with " + prefs.get("programmer.name", null));
                        return;
                      }
                    } else {
                      progPane.append("Fuse " + tabs[ii].toLowerCase() + " already set correctly, so left unchanged\n");
                    }
                  }
                } else {
                  progPane.append("Fuses not programmed\n");
                }
                // Copy contents of "hex" pane to temp file with .hex extension
                String hex = hexPane.getText();
                FileOutputStream fOut = new FileOutputStream(tmpDir + "code.hex");
                fOut.write(hex.getBytes(StandardCharsets.UTF_8));
                fOut.close();
                // Use AVRDUDE to program chip
                Map<String, String> tags = new HashMap<>();
                int retVal = callAvrdude("-U flash:w:*[TDIR]*code.hex", tags);
                if (retVal != 0) {
                  showErrorDialog("Error Programming Device Signature with " + prefs.get("programmer.name", null));
                }
              }
            } catch (Exception ex) {
              ex.printStackTrace();
              showErrorDialog(ex);
            }
          });
          tt.start();
        } else {
          showErrorDialog("Not supported by selected programmer and target");
        }
      } else {
        showErrorDialog("Target device type not selected");
      }
    });
    /*
     *    Read/Modify Fuse(s) Menu Item
     */
    actions.add(mItem = new JMenuItem("Read/Modify Fuse(s)"));
    mItem.setToolTipText("Commands Programmer to Read and optionally Modify Device's Fuse Byte(s)");
    mItem.addActionListener(e -> {
      if (avrChip != null) {
        ChipInfo info = progProtocol.get(avrChip);
        if ("TPI".equals(info.prog) && isSerialProgrammer() && !isAvrdudeProgrammer() && supportsTPI()) {
          // Read modify/fuses using ATTiny10GeneratedProgrammer sketch as TPI-based programmer
          try {
            String rsp = (new JPortSender("F\n", true)).rsp;
            if (rsp != null && rsp.startsWith("Fuse:")) {
              int idx = rsp.indexOf("0xF");
              String fuse =  rsp.substring(idx + 2, idx + 4);
              int fVal = Integer.parseInt(fuse, 16);
              ParmDialog.ParmItem[][] parmSet = getTPIFuseParms(fVal);
              String[] tabs = {"FUSE"};
              ParmDialog dialog = new ParmDialog(parmSet, tabs, new String[] {"Program", "Cancel"});
              dialog.setLocationRelativeTo(this);
              dialog.setVisible(true);              // Note: this call invokes dialog
              if (dialog.wasPressed()) {
                int val = dialog.fuseSet[0].fuseValue;
                fuse = Integer.toHexString(val & 0x0F).toUpperCase();
                selectTab(Tab.PROG);
                progPane.setText("");
                sendToJPort("\nD\n*" + fuse + "\n:00000001FF\n");
              }
            } else {
              showErrorDialog("Unable to read Fuse byte");
            }
          } catch (Exception ex) {
            ex.printStackTrace();
            showErrorDialog(ex);
          }
        } else if ("TPI".equals(info.prog) && isAvrdudeProgrammer() && supportsTPI()) {
          // Read modify/fuses using AVRDUDE as TPI-based programmer
          selectTab(Tab.PROG);
          progPane.setText("");
          Thread tt = new Thread(() -> {
            try {
              // Read current fuse settings from chip
              Map<String, String> tags = new HashMap<>();
              int retVal = callAvrdude("-U fuse:r:*[TDIR]*fuse.hex:h ", tags);
              if (retVal != 0) {
                showErrorDialog("Error Reading Fuse with " + prefs.get("programmer.name", null));
                return;
              }
              int fuse = Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*fuse.hex", tags)).trim());
              ParmDialog.ParmItem[][] parmSet = getTPIFuseParms(fuse);
              String[] tabs = {"FUSE"};
              ParmDialog dialog = new ParmDialog(parmSet, tabs, new String[] {"Program", "Cancel"});
              dialog.setLocationRelativeTo(this);
              dialog.setVisible(true);              // Note: this call invokes dialog
              if (dialog.wasPressed()) {
                int dFuse = dialog.fuseSet[0].fuseValue;
                if (fuse != dFuse) {
                  // Update fuse value
                  tags.put("FVAL", "0x" + Integer.toHexString(dFuse).toUpperCase());
                  retVal = callAvrdude("-U fuse:w:*[FVAL]*:m", tags);
                  if (retVal != 0) {
                    showErrorDialog("Error Writing Fuse with " + prefs.get("programmer.name", null));
                  }
                } else {
                  progPane.append("Fuse already set correctly, so left unchanged\n");
                }
             }
            } catch (Exception ex) {
              ex.printStackTrace();
              showErrorDialog(ex);
            }
          });
          tt.start();
        } else if ("ISP".equals(info.prog) && isAvrdudeProgrammer() && supportsISP()) {
          // Read modify/fuses using AVRDUDE as ISP-based programmer
          selectTab(Tab.PROG);
          progPane.setText("");
          Thread tt = new Thread(() -> {
            try {
              // Read current fuse settings from chip
              Map<String, String> tags = new HashMap<>();
              int retVal = callAvrdude("-U lfuse:r:*[TDIR]*lfuse.hex:h -U hfuse:r:*[TDIR]*hfuse.hex:h -U efuse:r:*[TDIR]*efuse.hex:h", tags);
              if (retVal != 0) {
                showErrorDialog("Error Reading Fuses with " + prefs.get("programmer.name", null));
                return;
              }
              int[] fuses = {
                  Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*lfuse.hex", tags)).trim()),
                  Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*hfuse.hex", tags)).trim()),
                  Integer.decode(Utility.getFile(Utility.replaceTags("*[TDIR]*efuse.hex", tags)).trim())
              };
              ParmDialog.ParmItem[][] parmSet = getSPIFuseParms(fuses[0], fuses[1], fuses[2]);
              String[] tabs = {"LFUSE", "HFUSE", "EFUSE"};
              ParmDialog dialog = new ParmDialog(parmSet, tabs, new String[]{"Program", "Cancel"});
              dialog.setLocationRelativeTo(this);
              dialog.setVisible(true);              // Note: this call invokes dialog
              if (dialog.wasPressed()) {
                for (int ii = 0; ii < fuses.length; ii++) {
                  int dFuse = dialog.fuseSet[ii].fuseValue;
                  int cFuse = fuses[ii];
                  if (dFuse != cFuse) {
                    // Update fuse value
                    tags.put("FUSE", tabs[ii].toLowerCase());
                    tags.put("FVAL", "0x" + Integer.toHexString(dFuse).toUpperCase());
                    retVal = callAvrdude("-U *[FUSE]*:w:*[FVAL]*:m", tags);
                    if (retVal != 0) {
                      showErrorDialog("Error Writing Fuses with " + prefs.get("programmer.name", null));
                      return;
                    }
                  } else {
                    progPane.append("Fuse " + tabs[ii].toLowerCase() + " already set correctly, so left unchanged\n");
                  }
                }
              }
            } catch (Exception ex) {
              ex.printStackTrace();
              showErrorDialog(ex);
            }
          });
          tt.start();
        } else {
          showErrorDialog("Not supported by selected programmer and target");
        }
      } else {
        showErrorDialog("Target device type not selected");
      }
    });
    /*
     *    Read Signature
     */
    actions.add(mItem = new JMenuItem("Read Device Signature"));
    mItem.setToolTipText("Commands Programmer to Read and Send Back Device's Signature");
    mItem.addActionListener(e -> {
      if (avrChip != null) {
        ChipInfo info = progProtocol.get(avrChip);
        if ("TPI".equals(info.prog) && isSerialProgrammer() && !isAvrdudeProgrammer() && supportsTPI()) {
          // Read signaure using ATTiny10GeneratedProgrammer sketch as TPI-based programmer
          try {
            selectTab(Tab.PROG);
            progPane.setText("Read Device Signature and Fuse\n");
            sendToJPort("S\n");
          } catch (Exception ex) {
            ex.printStackTrace();
            showErrorDialog(ex);
          }
        } else if ("TPI".equals(info.prog) && isAvrdudeProgrammer() && supportsTPI() ||
                   "ISP".equals(info.prog) && isAvrdudeProgrammer() && supportsISP()) {
          // Read signature using AVRDUDE and TPI or ISP protocol
          selectTab(Tab.PROG);
          progPane.setText("");
          Thread tt = new Thread(() -> {
            try {
              Map<String, String> tags = new HashMap<>();
              int retVal = callAvrdude("-U signature:r:*[TDIR]*sig.hex:h", tags);
              if (retVal != 0) {
                showErrorDialog("Error Reading Device Signature with " + prefs.get("programmer.name", null));
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
        } else {
          showErrorDialog("Not supported by selected programmer and target");
        }
      } else {
        showErrorDialog("Target device type not selected");
      }
    });
    /*
     *    Calibrate Clock
     */
    actions.add(mItem = new JMenuItem("Calibrate Device Clock"));
    mItem.setToolTipText("Commands TPI Programmer to Upload and Run Clock Calibration Code on Device");
    mItem.addActionListener(e -> {
      if (avrChip != null) {
        ChipInfo info = progProtocol.get(avrChip);
        if ("TPI".equals(info.prog) && isSerialProgrammer() && supportsTPI()) {
          if (doWarningDialog("Clock Calibration requires overwriting the code in the target device.  Proceed?")) {
            try {
              selectTab(Tab.PROG);
              progPane.setText("Programming Clock Code\n");
              String hex = Utility.getFile("res:clockcal.hex");
              sendToJPort("\nD\n" + hex + "\nM\n");
            } catch(Exception ex){
              ex.printStackTrace();
              showErrorDialog(ex);
            }
          }
        } else {
          showErrorDialog("Not supported by selected programmer and target");
        }
      }
    });
    /*
     *  Generate Arduino Programmer Code Menu
     */
    actions.addSeparator();
    actions.add(mItem = new JMenuItem("Generate Arduino Programmer Code"));
    mItem.setToolTipText("Generates Arduino Sketch that can Program Device with Built Code");
    mItem.addActionListener(e -> {
      if (avrChip != null) {
        ChipInfo info = progProtocol.get(avrChip);
        switch (info.prog) {
          case "TPI":
            if (canProgram()) {
              try {
                // Convert Intel Hex into byte array elements
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
                String genCode = Utility.getFile("res:ATTiny10GeneratedProgrammer.ino");
                String outCode = genCode.replace("/*[CODE]*/", buf.toString()).replace("/*[FUSE]*/", Character.toString(fuseCode));
                outCode = outCode.replace("/*[NAME]*/", cFile.getName());
                // Handle exported parameters, if any are declared using "#pragma xparm"
                StringBuilder parms = new StringBuilder();
                if (exportParms != null && exportParms.length() > 0) {
                  parms.append("\n  ");
                  StringTokenizer tok = new StringTokenizer(exportParms, "\n");
                  while (tok.hasMoreElements()) {
                    String line = tok.nextToken();
                    String[] parts = line.split(":");
                    if (parts.length == 3) {
                      String name = parts[0];
                      for (int ii = 0; ii < name.length(); ii++) {
                        parms.append("'");
                        parms.append(name.charAt(ii));
                        parms.append("',");
                      }
                      parms.append("0,");
                      parms.append(parts[2]);
                      parms.append(",0x");
                      int add = Integer.parseInt(parts[1], 16);
                      parms.append(Utility.hexChar((byte) (add >> 12)));
                      parms.append(Utility.hexChar((byte) (add >> 8)));
                      parms.append(",0x");
                      parms.append(Utility.hexChar((byte) (add >> 4)));
                      parms.append(Utility.hexChar((byte) (add & 0x0F)));
                      if (tok.hasMoreElements()) {
                        parms.append(",\n  ");
                      }
                    }
                  }
                  parms.append("\n");
                }
                outCode = outCode.replace("/*[PARMS]*/", parms.toString());
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
                ex.printStackTrace();
                showErrorDialog(ex);
              }
              break;
            }
            break;
          case "ISP":
            showErrorDialog("Not Implemented for ISP Protocol");
            break;
        }
      } else {
        showErrorDialog("Target device type not selected");
      }
    });
    if (prefs.getBoolean("developer_features", false)) {
      actions.addSeparator();
      actions.add(mItem = new JMenuItem("Power On"));
      mItem.setToolTipText("Commands TPI Programmer to Switch On Power to Device");
      mItem.addActionListener(e -> {
        try {
          progPane.setText("Enable Vcc to target\n");
          sendToJPort("V\n");
        } catch (Exception ex) {
          ex.printStackTrace();
          showErrorDialog(ex);
        }
      });
      actions.add(mItem = new JMenuItem("Power Off"));
      mItem.setToolTipText("Commands TPI Programmer to Switch Off Power to Device");
      mItem.addActionListener(e -> {
        try {
          progPane.setText("Disable Vcc to target\n");
          sendToJPort("X\n");
        } catch (Exception ex) {
          ex.printStackTrace();
          showErrorDialog(ex);
        }
      });
    }

    /*
     *    Reinstall Toolchain Menu Item
     */
    actions.addSeparator();
    actions.add(mItem = new JMenuItem("Reinstall Toolchain"));
    mItem.setToolTipText("Copies AVR Toolchain into Java Temporary Disk Space where it can be Executed");
    mItem.addActionListener(e -> reloadToolchain());
    menuBar.add(actions);
    /*
     *    Settings menu
     */
    JMenu settings = new JMenu("Settings");
    menuBar.add(settings);
    // Add "Tabs" Menu with submenu
    settings.add(codePane.getTabSizeMenu());
    settings.addSeparator();
    // Add "Font Size" Menu with submenu
    settings.add(codePane.getFontSizeMenu());
    settings.addSeparator();
    // Add Programmer Menu Item
    JMenuItem programmer = new JMenuItem("Programmer");
    programmer.addActionListener(ev -> {
      try {
        ProgrammerDialog dialog = new ProgrammerDialog(prefs);
        dialog.setLocationRelativeTo(this);
        dialog.setVisible(true);
        /*
        if (dialog.wasPressed()) {
          System.out.println("Save:");
          System.out.println("  programmer.name:       " + prefs.get("programmer.name", null));
          System.out.println("  programmer.programmer: " + prefs.get("programmer.programmer", null));
          System.out.println("  programmer.protocols:  " + prefs.get("programmer.protocols", null));
          System.out.println("  programmer.connection: " + prefs.get("programmer.connection", null));
          System.out.println("  programmer.port:       " + prefs.get("programmer.port", null));
          System.out.println("  programmer.rate:       " + prefs.get("programmer.rate", null));
        }
        */
      } catch (Exception ex) {
        ex.printStackTrace();
        showErrorDialog(ex);
      }
    });
    settings.add(programmer);
    /*
     *    Target Menu
     */
    targetMenu = new RadioMenu("Target");
    avrChip = prefs.get("programmer.target", "attiny10");
    ButtonGroup targetGroup = new ButtonGroup();
    menuBar.add(targetMenu);
    String libType = null;
    for (String type : progProtocol.keySet()) {
      ChipInfo info = progProtocol.get(type);
      if ((libType != null && !libType.equals(info.variant)) || (libType == null && info.variant != null)) {
        targetMenu.addSeparator();
      }
      libType = info.variant;
      JRadioButtonMenuItem item = new JRadioButtonMenuItem(type);
      if (avrChip.equals(type)) {
        item.setSelected(true);
      }
      targetMenu.add(item);
      targetGroup.add(item);
      item.addActionListener( ex -> {
        avrChip = type;
        targetMenu.setText("Target->" + avrChip);
        prefs.put("programmer.target", avrChip);
      });
    }
    targetMenu.setText("Target->" + avrChip);
    setJMenuBar(menuBar);
    // Add window close handler
    addWindowListener(new WindowAdapter() {
      public void windowClosing (WindowEvent ev) {
        if (!codeDirty  ||  discardChanges()) {
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
    verifyToolchain();
  }

  private boolean supportsTPI () {
    return supports("protocols", "TPI");
  }

  private boolean supportsISP () {
    return supports("protocols", "ISP");
  }

  private boolean isSerialProgrammer () {
    return supports("connection", "serial");
  }

  private boolean isAvrdudeProgrammer () {
    return supports("connection", "avrdude");
  }

  private boolean supports (String field, String match) {
    String value = prefs.get("programmer." + field, "");
    String[] parts = value.split(";");
    for (String part : parts) {
      if (match.equals(part)) {
        return true;
      }
    }
    return false;
  }

  private void updateChip (String src) {
    int idx = src.lastIndexOf("#pragma");
    int end = src.indexOf('\n', idx);
    String pragArea = src.substring(0, end + 1);
    for (String line : pragArea.split("\n")) {
      idx = line.indexOf("//");
      if (idx >= 0) {
        line = line.substring(0, idx).trim();
      }
      if (line.startsWith("#pragma")) {
        line = line.substring(7).trim();
        String[] parts = Utility.parse(line);
        if (parts.length > 1 && "chip".equals(parts[0])) {
          if (progProtocol.containsKey(parts[1])) {
            avrChip = parts[1];
            targetMenu.setSelected(avrChip);
            targetMenu.setText("Target->" + avrChip);
            targetMenu.setEnabled(false);
            return;
          }
        }
      }
    }
    // If "#pragma chip" not found, or value is invalid set avrChip to selected item in "Target" menu, if any
    targetMenu.setEnabled(true);
    avrChip = targetMenu.getSelected();
    targetMenu.setText("Target->" + avrChip);
  }

  private void sendToJPort (String send) throws Exception {
    if (prefs.get("programmer.port", null) != null && prefs.get("programmer.rate", null) != null) {
      new JPortSender(send, false);
    } else {
      showErrorDialog("Programmer serial port not configured");
    }
  }

  private class JPortSender implements Runnable, JSSCPort.RXEvent {
    private String            send, rsp ="";
    private JSSCPort          jPort;
    private int               timoutReset;
    private volatile int      timeout;
    private volatile int      setupState;

    JPortSender (String send, boolean getResponse) throws Exception {
      this.send = send;
      jPort = new JSSCPort();
      jPort.setPort(prefs.get("programmer.port", null));
      jPort.setRate(prefs.get("programmer.rate", null));
      timoutReset= this.timeout = 100;   // timeout is 10 seconds
      Thread thrd = new Thread(this);
      if (jPort.open(this)) {
        thrd.start();
        if (getResponse) {
          thrd.join();
        }
      }
    }

    public void rxChar (byte cc) {
      if (setupState < 2) {
        // Watch for ACK ACK sequence to signal sketch is ready for commands
        switch (setupState) {
          case 0:
            setupState = cc == (char) 0x06 ? 1 : 0;
            break;
          case 1:
            setupState = cc == (char) 0x06 ? 2 : 0;
            break;
        }
      } else {
        // Watch for ESC to signal end of commands
        if (cc == 0x1B) {
          timeout = 0;
          return;
        } else {
          rsp += (char) cc;
        }
      }
      timeout = timoutReset;
      // Copy only printable chars and LF
      if (progPane != null && (cc >= 0x20 || cc == '\n')) {
        String tmp = Character.toString((char) cc);
        progPane.append(tmp);
      }
    }

    public void run () {
      try {
        // Send STK_LEAVE_PROGMODE ('Q') in case Optiboot is bootloader (fast exit)
        jPort.sendString("Q\n");
        // Wait for bootloader to time out so it doesn't swallow command
        while (timeout > 0 && setupState < 2) {
          synchronized (this) {
            timeout--;
          }
          try {
            Thread.sleep(100);
          } catch (Exception ex) {
            ex.printStackTrace();
          }
        }

        timeout = timoutReset;
        jPort.sendString(send + '*');
        while (timeout > 0) {
          synchronized (this) {
            timeout--;
          }
          try {
            Thread.sleep(100);
          } catch (Exception ex) {
            ex.printStackTrace();
          }
        }
        if (jPort != null && jPort.isOpen()) {
          jPort.close();
        }
      } catch (Exception ex) {
        ex.printStackTrace();
      }
    }
  }

  private int callAvrdude (String op,  Map<String, String> tags) throws Exception {
    tags.put("VBS", prefs.getBoolean("developer_features", false) ? "-v" : "");   // Use "-v -v -v" to see all
    tags.put("PROG", prefs.get("programmer.programmer", null));
    tags.put("TDIR", tmpDir);
    ChipInfo chipInfo = progProtocol.get(avrChip != null ? avrChip.toLowerCase() : null);
    tags.put("CHIP", chipInfo != null ? chipInfo.part : "t85");
    tags.put("CFG", tmpExe + "etc" + fileSep + "avrdude.conf");
    if (isSerialProgrammer()) {
      String port = prefs.get("programmer.port", "");
      String rate = prefs.get("programmer.rate", "");
      tags.put("OUT", "-P " + port + (rate != null && rate.length() > 0  ? " -b " + rate : ""));
    } else {
      tags.put("OUT", "-P usb");
    }
    String exec;
    if ("ftdiprog".equals(prefs.get("programmer.programmer", null))) {
      tags.put("CFG2", tmpExe + "etc" + fileSep + "ftdiprog.conf");
      exec = Utility.replaceTags("avrdude *[VBS]* *[OUT]* -C *[CFG]* -C +*[CFG2]* -c *[PROG]* -p *[CHIP]* " + op, tags);
    } else {
      exec = Utility.replaceTags("avrdude *[VBS]* *[OUT]* -C *[CFG]* -c *[PROG]* -p *[CHIP]* " + op, tags);
    }
    String cmd = tmpExe + "bin" + fileSep + exec;
    System.out.println("Run: " + cmd);
    Process proc = Runtime.getRuntime().exec(cmd);
    Utility.runCmd(proc, progPane);
    return proc.waitFor();
  }

  private ParmDialog.ParmItem[][] getSPIFuseParms (int lFuse, int hFuse, int eFuse) {
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

  private ParmDialog.ParmItem[][] getTPIFuseParms (int fuse) {
    return new ParmDialog.ParmItem[][]{
    {
      new ParmDialog.ParmItem("*Not Used",                !Utility.bit(fuse, 7)),
      new ParmDialog.ParmItem("*Not Used",                !Utility.bit(fuse, 6)),
      new ParmDialog.ParmItem("*Not Used",                !Utility.bit(fuse, 5)),
      new ParmDialog.ParmItem("*Not Used",                !Utility.bit(fuse, 4)),
      new ParmDialog.ParmItem("*Not Used",                !Utility.bit(fuse, 3)),
      new ParmDialog.ParmItem("CKOUT{*[CKOUT]*}",         !Utility.bit(fuse, 2)),
      new ParmDialog.ParmItem("WDTON{*[WDTON]*}",         !Utility.bit(fuse, 1)),
      new ParmDialog.ParmItem("RSTDISBL{*[RSTDISBL]*}",   !Utility.bit(fuse, 0)),
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

  static class ProgressBar extends JFrame {
    private final JDialog       frame;
    private final JProgressBar  progress;

    ProgressBar (JFrame comp, String msg) {
      frame = new JDialog(comp);
      frame.setUndecorated(true);
      JPanel pnl = new JPanel(new BorderLayout());
      pnl.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
      frame.add(pnl, BorderLayout.CENTER);
      pnl.add(progress = new JProgressBar(), BorderLayout.NORTH);
      JTextArea txt = new JTextArea(msg);
      txt.setEditable(false);
      pnl.add(txt, BorderLayout.SOUTH);
      Rectangle loc = comp.getBounds();
      frame.pack();
      frame.setLocation(loc.x + loc.width / 2 - 150, loc.y + loc.height / 2 - 150);
      frame.setVisible(true);
    }

    void setValue (int value) {
      SwingUtilities.invokeLater(() -> progress.setValue(value));
    }

    void setMaximum (int value) {
      progress.setMaximum(value);
    }

    void close () {
      frame.setVisible(false);
      frame.dispose();
    }
  }

  /**
   * Very all the files in the toolchain are intact by computing a CRC2 value from the tree
   * of directory and file names.  Note: the CRC is not based on the content of the files.
   */
  private void verifyToolchain () {
    boolean reloadTools = prefs.getBoolean("reload_toolchain", false);
    if (!reloadTools) {
      long oldCrc = prefs.getLong("toolchain-crc", 0);
      long newCrc = Utility.crcTree(tmpExe);
      reloadTools = newCrc != oldCrc;
    }
    if (!reloadTools) {
      // Check for new toolchain
      String zipFile = null;
      if (os == OpSys.WIN) {
        zipFile = "toolchains/WinToolchain.zip";
      } else if (os == OpSys.MAC) {
        zipFile = "toolchains/MacToolchain.zip";
      } else if (os == OpSys.LINUX) {
        zipFile = "toolchains/L64Toolchain.zip";
      }
      long oldCrc = prefs.getLong("toolzip-crc", 0);
      long newCrc = Utility.crcZipfile(zipFile);
      reloadTools = newCrc != oldCrc;
      prefs.putLong("toolzip-crc", newCrc);
    }

    if (reloadTools) {
      reloadToolchain();
    }
  }

  private void reloadToolchain () {
    try {
      if (os == OpSys.WIN) {
        loadToolchain("toolchains/WinToolchain.zip");
      } else if (os == OpSys.MAC) {
        loadToolchain("toolchains/MacToolchain.zip");
      } else if (os == OpSys.LINUX) {
        loadToolchain("toolchains/L64Toolchain.zip");
      }
      /*
      // Create and save ftdipro.conf file
      String ftdiprog = "programmer\n" +
          "  id    = \"ftdiprog\";\n" +
          "  desc  = \"design ftdi adatper, reset=rts sck=dtr mosi=txd miso=cts\";\n" +
          "  type  = \"serbb\";\n" +
          "  miso  = ~8;\n" +         // CTS (inverted)
          "  reset = ~7;\n" +         // RTS (inverted)
          "  sck   = ~4;\n" +         // DTR (inverted)
          "  mosi  = ~3;\n" +         // TxD (inverted)
          ";\n";
      Utility.saveFile(tmpExe + "etc" + fileSep + "ftdiprog.conf", ftdiprog);
       */
      // Compute CRC for toolchain
      prefs.remove("reload_toolchain");
      prefs.putLong("toolchain-crc", Utility.crcTree(tmpExe));
    } catch (Exception ex) {
      ex.printStackTrace();
      selectTab(Tab.LIST);
      listPane.setText("Unable to Install Toolchain:\n" + ex.toString());
    }
  }

  private void loadToolchain (String src) {
    new ToolchainLoader(this, src, tmpExe);
  }

  class ToolchainLoader implements Runnable  {
    private String            srcZip, tmpExe;
    private ProgressBar       progress;

    ToolchainLoader (JFrame comp, String srcZip, String tmpExe) {
      progress = new ProgressBar(comp, "Installing AVR Toolchain");
      this.srcZip = srcZip;
      this.tmpExe = tmpExe;
      new Thread(this).start();
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
          InputStream stream = ATTinyC.class.getClassLoader().getResourceAsStream(srcZip);
          if (stream != null) {
            Files.copy(stream, file, StandardCopyOption.REPLACE_EXISTING);
            File srcFile = file.toFile();
            srcFile.deleteOnExit();
            zip = new ZipFile(srcFile);
            int entryCount = 0, lastEntryCount = 0;
            progress.setMaximum(zip.size());
            Enumeration entries = zip.entries();
            while (entries.hasMoreElements()) {
              ZipEntry entry = (ZipEntry) entries.nextElement();
              entryCount++;
              if (entryCount - lastEntryCount > 100) {
                progress.setValue(lastEntryCount = entryCount);
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
          } else {
            showErrorDialog("Unable to open " + srcZip + file);
          }
        } finally {
          if (zip != null) {
            zip.close();
          }
        }
      } catch (Exception ex) {
        ex.printStackTrace();
      }
      progress.close();
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

  private void showErrorDialog (Exception ex) {
    String msg = ex.getMessage();
    if (msg != null && msg.length() > 0) {
      showErrorDialog(msg);
    } else {
      msg = ex.toString();
      showErrorDialog(msg);
    }
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
