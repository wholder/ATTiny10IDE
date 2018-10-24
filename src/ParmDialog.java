import java.awt.*;
import java.util.Properties;
import javax.swing.*;
import javax.swing.border.TitledBorder;

import static javax.swing.JOptionPane.showMessageDialog;

class ParmDialog extends JDialog {
  private static final double[]   lblWeights = {0.6, 0.3, 0.1};
  private static Properties       fuseInfo;
  private boolean                 cancelled = true;
  ParmPanel[]                     fuseSet;

  static {
    try {
      fuseInfo = Utility.getResourceMap("dialoginfo.props");
    } catch (Exception ex) {
      ex.printStackTrace();
    }
  }

  static class ParmItem {
    String      name, info;
    boolean     value, readOnly, warn;

    ParmItem (String name, boolean value) {
      name = Utility.replaceTags(name, fuseInfo);
      int idx1 = name.indexOf("{");
      int idx2 = name.indexOf("}");
      if (idx1 >= 0 && idx2 >= 0 && idx2 > idx1) {
        // Setup Info button
        info = name.substring(idx1 + 1, idx2);
        name = name.replace("{" + info + "}", "");
      }
      if (name.startsWith("*")) {
        // Set Read Only
        name = name.substring(1);
        readOnly = true;
      }
      if (name.startsWith("!")) {
        // Set Read Only
        name = name.substring(1);
        warn = true;
      }
      this.name = name;
      this.value = value;
    }
  }

  private GridBagConstraints getGbc (int x, int y) {
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.gridx = x;
    gbc.gridy = y;
    gbc.anchor = (x == 0) ? GridBagConstraints.WEST : GridBagConstraints.EAST;
    gbc.fill = (x == 0) ? GridBagConstraints.BOTH : GridBagConstraints.HORIZONTAL;
    gbc.weightx = lblWeights[x];
    gbc.ipady = 2;
    return gbc;
  }

  boolean wasPressed () {
    return !cancelled;
  }

  /**
   * Constructor for Pop Up Parameters Dialog with error checking
   * @param parms array of ParmItem objects that describe each parameter
   * @param tabs String array of names for the tags
   * @param buttons String] array of button names (first name in array is action button)
   */
  ParmDialog (ParmItem[][] parms, String[] tabs, String[] buttons) {
    super((Frame) null, true);
    fuseSet = new ParmPanel[parms.length];
    setTitle("Fuse Settings");
    setLayout(new BorderLayout(1, 1));
    JComponent parmPane;
    add(parmPane = new JPanel());
    parmPane.setLayout(new GridLayout(1, tabs != null ? tabs.length : 1));
    for (int ii = 0; ii < parms.length; ii++) {
      ParmPanel fuses = fuseSet[ii] = new ParmPanel(parms[ii], tabs != null ? tabs[ii] : null);
      parmPane.add(fuses);
    }
    add("Center", parmPane);
    JOptionPane optionPane = new JOptionPane(parmPane, JOptionPane.PLAIN_MESSAGE, JOptionPane.YES_NO_OPTION, null, buttons, buttons[0]);
    setContentPane(optionPane);
    setDefaultCloseOperation(DISPOSE_ON_CLOSE);
    optionPane.addPropertyChangeListener(ev -> {
      String prop = ev.getPropertyName();
      if (isVisible() && (ev.getSource() == optionPane) && (JOptionPane.VALUE_PROPERTY.equals(prop) ||
        JOptionPane.INPUT_VALUE_PROPERTY.equals(prop))) {
        Object value = optionPane.getValue();
        if (value != JOptionPane.UNINITIALIZED_VALUE) {
          // Reset the JOptionPane's value.  If you don't do this, then if the user
          // presses the same button next time, no property change event will be fired.
          optionPane.setValue(JOptionPane.UNINITIALIZED_VALUE);
          if (buttons[0].equals(value)) {
            cancelled = false;
          }
          dispose();
        }
      }
    });
    pack();
    setResizable(false);
  }

  class ParmPanel extends JPanel {
    ParmItem[]    parms;
    String        tabName;
    int           fuseValue;
    TitledBorder  bdr;

    ParmPanel (ParmItem[] parms, String tabName) {
      this.parms = parms;
      this.tabName = tabName;
      setLayout(new GridBagLayout());
      int jj = 0;
      for (ParmItem parm : parms) {
        int row = jj;
        JLabel lbl = new JLabel(parm.name + ": ");
        if (parm.warn) {
          lbl.setForeground(Color.red);
        }
        add(lbl, getGbc(0, jj));
        JCheckBox select = new JCheckBox();
        select.setEnabled(!parm.readOnly);
        select.setBorderPainted(false);
        select.setFocusable(false);
        select.setBorderPaintedFlat(true);
        select.setSelected(parm.value);
        select.setHorizontalAlignment(JCheckBox.RIGHT);
        add(select, getGbc(1, jj));
        select.addActionListener(ex -> {
          if (parm.warn) {
            if (!okToChange()) {
              select.setSelected(!select.isSelected());
            }
          }
          parm.value = select.isSelected();
          if (tabName != null) {
            fuseValue = (fuseValue & ~(0x80 >> row)) | (parm.value ? 0 : 0x80 >> row);
            bdr.setTitle(tabName + " = 0x" + (fuseValue < 16 ? "0" : "") + Integer.toHexString(fuseValue).toUpperCase());
            repaint();
          }
        });
        int col = 2;
        if (parm.info != null) {
          // Add popup Info dialog
          try {
            final String[] tmp = parm.info.split("--");
            ImageIcon icon = new ImageIcon(getClass().getResource("/images/info.png"));
            JButton iBut = new JButton(icon);
            Dimension dim = iBut.getPreferredSize();
            iBut.setPreferredSize(new Dimension(dim.width - 4, dim.height - 4));
            add(iBut, getGbc(col, jj));
            iBut.addActionListener(ev -> {
              JEditorPane textArea = new JEditorPane();
              textArea.setContentType("text/html");
              textArea.putClientProperty(JEditorPane.HONOR_DISPLAY_PROPERTIES, Boolean.TRUE);
              textArea.setFont(new Font("Arial", Font.PLAIN, 14));
              JScrollPane scrollPane = new JScrollPane(textArea);
              scrollPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
              String iMsg = tmp.length > 1 ? tmp[1] : tmp[0];
              textArea.setText(iMsg);
              textArea.setEditable(false);
              textArea.setCaretPosition(0);
              scrollPane.setPreferredSize(new Dimension(350, 150));
              showMessageDialog(this, scrollPane, tmp.length > 1 ? tmp[0] : parm.name, JOptionPane.PLAIN_MESSAGE);
            });
          } catch (Exception ex) {
            ex.printStackTrace();
          }
        } else {
          add(new JLabel(""), getGbc(col, jj));
        }
        if (tabName != null) {
          fuseValue |= (parm.value ? 0 : 0x80) >> row;
        }
        jj++;
      }
      if (tabName != null) {
        bdr = new TitledBorder("");
        bdr.setTitleJustification(TitledBorder.CENTER);
        bdr.setTitle(tabName + " = 0x" + (fuseValue < 16 ? "0" : "") + Integer.toHexString(fuseValue).toUpperCase());
        setBorder(bdr);
      }
    }

    private boolean okToChange () {
      return JOptionPane.showConfirmDialog(this, "Are you sure you want to change this fuse?", "Warning", JOptionPane.YES_NO_OPTION,
        JOptionPane.WARNING_MESSAGE, null) == JOptionPane.OK_OPTION;
    }
  }

  public static void main (String... args) {
    ParmItem[][] parmSet = {
      {
        new ParmItem("CKDIV8{*[CKDIV8]*}", true),
        new ParmItem("CKOUT{*[CKOUT]*}", false),
        new ParmItem("SUT1{*[SUT]*}", false),
        new ParmItem("SUT0{*[SUT]*}", true),
        new ParmItem("CKSEL3{*[CKSEL]*}", true),
        new ParmItem("CKSEL2{*[CKSEL]*}", true),
        new ParmItem("CKSEL1{*[CKSEL]*}", true),
        new ParmItem("CKSEL0{*[CKSEL]*}", true),
      }, {
        new ParmItem("!RSTDISBL{*[RSTDISBL]*}", false),
        new ParmItem("!DWEN{*[DWEN]*}", false),
        new ParmItem("!SPIEN{*[SPIEN]*}", true),
        new ParmItem("WDTON{*[WDTON]*}", false),
        new ParmItem("EESAVE{*[EESAVE]*}", false),
        new ParmItem("BODLEVEL2{*[BODLEVEL]*}", false),
        new ParmItem("BODLEVEL1{*[BODLEVEL]*}", false),
        new ParmItem("BODLEVEL0{*[BODLEVEL]*}", false),
      }, {
        new ParmItem("*Not Used", false),
        new ParmItem("*Not Used", false),
        new ParmItem("*Not Used", false),
        new ParmItem("*Not Used", false),
        new ParmItem("*Not Used", false),
        new ParmItem("*Not Used", false),
        new ParmItem("*Not Used", false),
        new ParmItem("SELFPRGEN{*[SELFPRGEN]*}", false),
      }
    };
    String[] tabs = {"LFUSE", "HFUSE", "EFUSE"};
    ParmDialog dialog = new ParmDialog(parmSet, tabs, new String[] {"Save", "Cancel"});
    dialog.setLocationRelativeTo(null);
    dialog.setVisible(true);              // Note: this call invokes dialog
    if (dialog.wasPressed()) {
      for (ParmPanel fp : dialog.fuseSet) {
        int val = fp.fuseValue;
        System.out.println(fp.tabName + " = 0x" + (val < 16 ? "0" : "") + Integer.toHexString(val).toUpperCase());
      }
    } else {
      System.out.println("Cancel");
    }
  }
}