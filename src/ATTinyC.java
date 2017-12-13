import java.awt.*;
import java.awt.event.*;

import java.io.*;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.channels.ReadableByteChannel;
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
   *    Program abd debug? See: http://www.ruemohr.org/docs/debugwire.html
   */

public class ATTinyC extends JFrame implements JSSCPort.RXEvent {
  private static Font               tFont = new Font("Monospaced", Font.PLAIN, 12);
  private static int                cmdMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
  private static KeyStroke          OPEN_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_O, cmdMask) ;
  private static KeyStroke          SAVE_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_S, cmdMask) ;
  private static KeyStroke          QUIT_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_Q, cmdMask) ;
  private static KeyStroke          BUILD_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_B, cmdMask) ;
  private static KeyStroke          PROG_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_P, cmdMask) ;
  private static Map<String,String> progProtocol = new HashMap<>();
  private enum                      Tab {SRC(0), LIST(1), HEX(2), PROG(3), INFO(4); final int num; Tab(int num) {this.num = num;}};
  private String                    osName = System.getProperty("os.name").toLowerCase();
  private JTabbedPane               tabPane;
  private JFileChooser              fc = new JFileChooser();
  private CodeEditPane              codePane;
  private JTextArea                 listPane, hexPane, progPane, infoPane;
  private JMenuItem                 saveMenu;
  private String                    tmpDir, tmpExe, chip;
  private boolean                   directHex, compiled, codeDirty;
  private File                      cFile;
  private ATTiny10Compiler          compiler = new ATTiny10Compiler();
  private transient Preferences     prefs = Preferences.userNodeForPackage(getClass());
  private transient JSSCPort        jPort;

  static {
    // List of ATtiny types and the Protocol used to Program them.
    progProtocol.put("attiny4", "TPI");
    progProtocol.put("attiny5", "TPI");
    progProtocol.put("attiny9", "TPI");
    progProtocol.put("attiny10", "TPI");
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
  }

  private void selectTab (Tab tab) {
    tabPane.setSelectedIndex(tab.num);
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
      codeDirty = true;
      compiled = false;
      listPane.setForeground(Color.red);
      hexPane.setForeground(Color.red);
    });
    tabPane.addTab("Source Code", null, codePane, "This is the editor pane where you enter source code");
    listPane = getScrollingTextPage(tabPane, "Listing", "Select this pane to view the assembler listing");
    hexPane = getScrollingTextPage(tabPane, "Hex Output", "Intel Hex Output file for programmer");
    progPane = getScrollingTextPage(tabPane, "Programmer", "Records communication with Arduino-based programmer");
    // Add System Info Tab
    infoPane = getScrollingTextPage(tabPane, "System Info", "Displays information about System OS");
    infoPane.append("os.name: " + osName + "\n");
    // Add menu bar and menus
    JMenuBar menuBar = new JMenuBar();
    // Add "File" Menu
    JMenu fileMenu = new JMenu("File");
    JMenuItem mItem;
    fileMenu.add(mItem = new JMenuItem("About"));
    mItem.addActionListener(e -> JOptionPane.showMessageDialog(this, "By: Wayne Holder",
        "ATtiny10IDE", JOptionPane.PLAIN_MESSAGE, new ImageIcon(getClass().getResource("images/tiny10-128x128.jpg"))));
    fileMenu.addSeparator();
    fileMenu.add(mItem = new JMenuItem("New"));
    mItem.addActionListener(e -> {
      if (!codeDirty  ||  discardChanges()) {
        codePane.setText("");
        codePane.setForeground(Color.black);
        directHex = false;
        compiled = false;
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
            String tmp = getFile(oFile);
            if (tmp.contains(":00000001FF")) {
              if (!tmp.startsWith(":020000020000FC")) {
                tmp = ":020000020000FC\n" + tmp;
              }
              hexPane.setText(tmp);
              hexPane.setForeground(Color.black);
              selectTab(Tab.HEX);
              directHex = true;
            } else {
              cFile = oFile;
              codePane.setText(tmp);
              codePane.setForeground(Color.black);
              this.setTitle("ATTinyC: " + oFile.getAbsolutePath());
              codeDirty = false;
              directHex = false;
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
      saveFile(cFile, codePane.getText());
      codeDirty = false;
    });
    fileMenu.add(mItem = new JMenuItem("Save As..."));
    mItem.addActionListener(e -> {
      fc.setSelectedFile(new File(prefs.get("default.dir", "/")));
      if (fc.showSaveDialog(this) == JFileChooser.APPROVE_OPTION) {
        File sFile = fc.getSelectedFile();
        prefs.put("default.extension", ((FileNameExtensionFilter) fc.getFileFilter()).getExtensions()[0]);
        if (sFile.exists()) {
          if (doWarningDialog("Overwrite Existing file?")) {
            saveFile(sFile, codePane.getText());
          }
        } else {
          saveFile(sFile, codePane.getText());
        }
        cFile = sFile;
        prefs.put("default.dir", sFile.getAbsolutePath());
        this.setTitle("ATTinyC: " + sFile.getAbsolutePath());
        codeDirty = false;
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
          listPane.setText(asm.getListing());
          listPane.setForeground(Color.black);
          hexPane.setText(asm.getHex());
          hexPane.setForeground(Color.black);
          compiled = true;
        } else {
          Thread cThread = new Thread(() -> {
            try {
              boolean doAsm = fName.endsWith(".s");
              Map<String, String> ret = compiler.compile(codePane.getText(), tmpExe, tmpDir, doAsm);
              if (ret.containsKey("ERR")) {
                listPane.setText(ret.get("ERR"));
                listPane.setForeground(Color.red);
                compiled = false;
              } else {
                listPane.setText(ret.get("INFO") + "\n\n" + ret.get("SIZE") + ret.get("LST"));
                listPane.setForeground(Color.black);
                hexPane.setText(ret.get("HEX"));
                hexPane.setForeground(Color.black);
                chip = ret.get("CHIP");
                compiled = true;
              }
            } catch (Exception ex) {
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
    actions.add(mItem = new JMenuItem("Program Device"));
    mItem.setAccelerator(PROG_KEY);
    mItem.addActionListener(e -> {
      try {
        if (canProgram()) {
          String hex = hexPane.getText();
          if (jPort != null && jPort.isOpen()) {
            selectTab(Tab.PROG);
            progPane.append("\nProgramming " + cFile.getName());
            jPort.sendString("\nD\n" + hex + "\n");
          } else {
            showErrorDialog("Serial port not selected!");
          }
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    actions.addSeparator();
    actions.add(mItem = new JMenuItem("Generate Arduino Programmer Code"));
    mItem.addActionListener(e -> {
      if (canProgram()) {
        try {
          String genCode = new String(getFile("res:ATTiny10GeneratedProgrammer.ino"));
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
            buf.append(ATTiny10Compiler.hexChar((byte) (code[ii] >> 4)));
            buf.append(ATTiny10Compiler.hexChar(code[ii]));
          }
          char fuseCode = ATTiny10Compiler.hexChar(image.fuses);
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
                saveFile(sFile, outCode);
              }
            } else {
              saveFile(sFile, outCode);
            }
          }
        } catch (Exception ex) {
          showErrorDialog(ex.getMessage());
        }
      }
    });
    actions.add(mItem = new JMenuItem("Calibrate Clock"));
    mItem.addActionListener(e -> {
      try {
        if (jPort != null && jPort.isOpen()) {
          selectTab(Tab.PROG);
          String hex = new String(getFile("res:clockcal.hex"));
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
    actions.add(mItem = new JMenuItem("Device Signature"));
    mItem.addActionListener(e -> {
      try {
        if (jPort != null && jPort.isOpen()) {
          selectTab(Tab.PROG);
          jPort.sendString("S");
        } else {
          showErrorDialog("Serial port not selected!");
        }
      } catch (Exception ex) {
        showErrorDialog(ex.getMessage());
      }
    });
    actions.add(mItem = new JMenuItem("Reinstall Toolchain"));
    mItem.addActionListener(e -> installToolchain(true));
    menuBar.add(actions);
    // Add Settings menu
    JMenu settings = new JMenu("Settings");
    settings.setEnabled(false);
    Thread oprtThread = new Thread(() -> {
      // Add "Port" and "Baud" Menus to MenuBar
      try {
        jPort = new JSSCPort(prefs);
        settings.add(jPort.getPortMenu());
        settings.add(jPort.getBaudMenu());
        jPort.setRXHandler(this);
      } catch (Exception ex) {
        ex.printStackTrace();
      } finally {
        settings.setEnabled(true);
      }
    });
    oprtThread.start();
    menuBar.add(settings);
    setJMenuBar(menuBar);
    // Disable menus unless "Source Code" tab is selected
    tabPane.addChangeListener(evt -> fileMenu.setEnabled(((JTabbedPane) evt.getSource()).getSelectedIndex() == Tab.SRC.num));
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
    installToolchain(false);
  }

  private boolean canProgram () {
    if (compiled || directHex) {
      // Check if code currently supports the protocol needed to program the chip
      if (chip != null && "TPI".equals(progProtocol.get(chip.toLowerCase()))) {
        return true;
      }
      showErrorDialog("Unable to program " + chip);
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
          new ToolchainLoader(this, "/WinToolchain.zip", tmpExe);
        } else if (osName.contains("mac")) {
          new ToolchainLoader(this, "/MacToolchain.zip", tmpExe);
        } else {
          throw new IllegalStateException("Unsupported os: " + osName);
        }
      } else {
        tmpExe = tmp.getAbsolutePath() + System.getProperty("file.separator");
      }
    } catch (Exception ex) {
      ex.printStackTrace();
    }
    infoPane.append("tmpDir:  " + tmpDir + "\n");
    infoPane.append("tmpExy:  " + tmpExe + "\n");
    infoPane.append("Java Simple Serial Connector: " + SerialNativeInterface.getLibraryVersion() + "\n");
    infoPane.append("JSSC Native Code DLL Version: " + SerialNativeInterface.getNativeLibraryVersion() + "\n");
  }

  class ToolchainLoader extends Thread  {
    private JDialog       frame;
    private JProgressBar  progress;
    private String        srcZip, tmpExe;

    ToolchainLoader (JFrame comp, String srcZip, String tmpExe) {
      this.srcZip = srcZip;
      this.tmpExe = tmpExe;
      frame = new JDialog(comp, "Loading AVR Toolchain");
      frame.setUndecorated(true);
      JPanel pnl = new JPanel(new GridLayout(2, 1, 5, 5));
      pnl.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
      frame.add(pnl, BorderLayout.CENTER);
      pnl.add(progress = new JProgressBar());
      JTextArea txt = new JTextArea("Loading AVR Toolchain,\n  Please wait.");
      txt.setEditable(false);
      pnl.add(txt);
      Rectangle loc = comp.getBounds();
      frame.setSize(200, 100);
      frame.setLocation(loc.x + loc.width / 2 - 150, loc.y + loc.height / 2 - 150);
      frame.setVisible(true);
      start();
    }

    public void run () {
      try {
        File dst = new File(tmpExe);
        if (!dst.exists()) {
          dst.mkdir();
        }
        String path = getClass().getResource(srcZip).getPath();
        ZipFile zip = null;
        try {
          try {
            zip = new ZipFile(new File(path));
          } catch (FileNotFoundException ex) {
            // Workaround for Windows unable to make a File() from a resource path
            Path file = Files.createTempFile(null, ".zip");
            InputStream stream = this.getClass().getResourceAsStream(srcZip);
            Files.copy(stream, file, StandardCopyOption.REPLACE_EXISTING);
            File srcFile = file.toFile();
            srcFile.deleteOnExit();
            zip = new ZipFile(srcFile);
          }
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
              dstDir.mkdir();
            }
            if (entry.isDirectory()) {
              dstFile.mkdirs();
            } else {
              ReadableByteChannel srcChan = null;
              FileChannel dstChan = null;
              try {
                srcChan = Channels.newChannel(zip.getInputStream(entry));
                dstChan = new FileOutputStream(dstFile).getChannel();
                dstChan.transferFrom(srcChan, 0, entry.getSize());
              } finally {
                if (srcChan != null) {
                  srcChan.close();
                }
                if (dstChan != null) {
                  dstChan.close();
                }
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
      frame.setVisible(false);
      frame.dispose();
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

  private String getFile (File file) throws IOException {
   FileInputStream fis = new FileInputStream(file);
   byte[] data = new byte[fis.available()];
   fis.read(data);
   fis.close();
   return new String(data, "UTF8");
  }
  
  private void saveFile (File file, String text) {
    try {
      FileOutputStream out = new FileOutputStream(file);
      out.write(text.getBytes("UTF8"));
      out.close();
    } catch (IOException ex) {
      ex.printStackTrace();
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
          int msn = fromHex(line.charAt(ii));
          int lsn = fromHex(line.charAt(ii + 1));
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
        fuses = (byte) fromHex(line.charAt(1));
      }
    }
    byte[] code = new byte[buf.size()];
    for (int ii = 0; ii < code.length; ii++) {
      code[ii] = buf.get(ii);
    }
    return new CodeImage(code, fuses);
  }

  private static int fromHex (char cc) {
    cc = Character.toUpperCase(cc);
    return cc >= 'A' ? cc - 'A' + 10 : cc - '0';
  }

  private byte[] getFile (String file) throws IOException {
    InputStream fis = file.startsWith("res:") ? getClass().getResourceAsStream("/" + file.substring(4)) : new FileInputStream(file);
    byte[] data = new byte[fis.available()];
    fis.read(data);
    fis.close();
    return data;
  }
  
  private boolean discardChanges () {
    return doWarningDialog("Discard Changes?");
  }

  private void showErrorDialog (String msg) {
    ImageIcon icon = new ImageIcon(getClass().getResource("images/warning-32x32.png"));
    JOptionPane.showMessageDialog(this, msg, "Error", JOptionPane.PLAIN_MESSAGE,
        icon);
  }

  private boolean doWarningDialog (String question) {
    ImageIcon icon = new ImageIcon(getClass().getResource("images/warning-32x32.png"));
    return JOptionPane.showConfirmDialog(this, question, "Warning", JOptionPane.YES_NO_OPTION,
                                         JOptionPane.WARNING_MESSAGE, icon) == JOptionPane.OK_OPTION;
  }

  public static void main (String[] args) {
    java.awt.EventQueue.invokeLater(ATTinyC::new);
  }
}
