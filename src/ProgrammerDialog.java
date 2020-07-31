import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.ActionListener;
import java.io.InputStream;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.prefs.Preferences;

public class ProgrammerDialog extends JDialog {
  private final Map<String,Map<String,String>>  progList;
  private boolean                               cancelled = true, firstSelect = true;
  private final DropDown                        progMenu;
  private final DropDown                        progPorts;
  private final DropDown                        progRates;
  private final JLabel                          progIcon;

  static class DropDown extends JComboBox<String> implements ListCellRenderer<String> {
    private final DefaultComboBoxModel<String>  model;
    private final Map<String,JLabel>            items = new HashMap<>();

    DropDown () {
      super(new DefaultComboBoxModel<>());
      model = (DefaultComboBoxModel<String>) getModel();
      setRenderer(this);
    }

    public JLabel getListCellRendererComponent (JList list, String value, int index, boolean isSelected, boolean cellHasFocus) {
      if (value != null) {
        JLabel lbl = items.get(value);
        if (isSelected) {
          lbl.setBackground(list.getSelectionBackground());
          lbl.setForeground(list.getSelectionForeground());
        } else {
          lbl.setBackground(list.getBackground());
          lbl.setForeground(list.getForeground());
        }
        return lbl;
      }
      return new JLabel("- -", SwingConstants.CENTER);
    }

    void setList (String[] list) {
      model.removeAllElements();
      if (list != null && list.length > 0) {
        for (String item : list) {
          addToList(item, item);
        }
      }
      setEnabled(list != null && list.length > 0);
    }

    void addToList(String label, String tooltip) {
      JLabel lbl = new JLabel(label);
      lbl.setBorder(BorderFactory.createEmptyBorder(2, 6, 0, 6));
      if (tooltip != null) {
        lbl.setToolTipText("<html>" + tooltip + "</html>");
      }
      items.put(label, lbl);
      lbl.setOpaque(true);
      lbl.setHorizontalAlignment(SwingConstants.LEFT);
      lbl.setVerticalAlignment(SwingConstants.CENTER);
      model.addElement(label);
      setMaximumRowCount(Math.max(10, model.getSize()));
    }
  }

  private void setIcon (String name) {
    try {
      URL url = getClass().getResource("images/programmers/" + name);
      if (url != null) {
        ImageIcon icon = new ImageIcon(url);
        progIcon.setIcon(icon);
        progIcon.repaint();
      } else {
        progIcon.setIcon(null);
      }
    } catch (Exception ex) {
      ex.printStackTrace();
    }
  }

  ProgrammerDialog (Preferences prefs) throws Exception {
    super((Frame) null, true);
    setTitle("Programmer");
    // Load programmer types file
    progList = new LinkedHashMap<>();
    InputStream fis = ProgrammerDialog.class.getClassLoader().getResourceAsStream("icsp_programmers.props");
    if (fis != null) {
      byte[] data = new byte[fis.available()];
      fis.read(data);
      fis.close();
      StringTokenizer tok = new StringTokenizer(new String(data, StandardCharsets.UTF_8), "}");
      while (tok.hasMoreElements()) {
        String line = Utility.condenseWhitespace(tok.nextToken().replace('\n', ' ')) + "}";
        String[] parts = line.split(":");
        if (parts.length == 2) {
          Map<String,String> map =  Utility.parseJSON(parts[1].trim());
          progList.put(parts[0].trim(), map);
        }
      }
    } else {
      throw new IllegalStateException("ProgrammerDialog() unable to read icsp_programmers.props");
    }
    // Setup GUI
    setLayout(new BorderLayout(1, 1));
    JPanel iconPanel = new JPanel(new BorderLayout());
    progIcon = new JLabel();
    progIcon.setPreferredSize(new Dimension(232,175));
    progIcon.setBorder(BorderFactory.createLineBorder(Color.black, 1));
    iconPanel.add(progIcon, BorderLayout.WEST);
    // Add padding panel for spacing between icon and controls panels
    JPanel pad = new JPanel();
    pad.setPreferredSize(new Dimension(5, 2));
    iconPanel.add(pad, BorderLayout.CENTER);
    // Add Controls panel
    JPanel controls = new JPanel(new GridLayout(6, 1));
    controls.setPreferredSize(new Dimension(300, 175));
    controls.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createLineBorder(Color.BLACK, 1), new EmptyBorder(5,5,5,5)));
    // Add Programmers Menu
    controls.add(new JLabel("Select Programmer:"));
    progMenu = new DropDown();
    for (String label : progList.keySet()) {
      Map<String, String> json = progList.get(label);
      progMenu.addToList(label, json.get("desc"));
    }
    controls.add(progMenu);
    controls.add(new JLabel("Select Port:"));
    progPorts = new DropDown();
    progPorts.setEnabled(false);
    controls.add(progPorts);
    controls.add(new JLabel("Select Baud Rate:"));
    progRates = new DropDown();
    progRates.setEnabled(false);
    controls.add(progRates);
    // Handle selection of a programmer and dependent dropdowns
    progMenu.addActionListener(ev -> {
      String programmer = (String) progMenu.getSelectedItem();
      Map<String, String> json = progList.get(programmer);
      setIcon(json.get("image"));
      String connection = json.get("connection");
      if (connection != null && connection.contains("serial")) {
        for (ActionListener listener : progPorts.getActionListeners()) {
          progPorts.removeActionListener(listener);
        }
        for (ActionListener listener : progRates.getActionListeners()) {
          progRates.removeActionListener(listener);
        }
        String[] ports = JSSCPort.getPortNames();
        progPorts.setList(ports);
        String rates = json.get("rates");
        String[] rateList = rates != null ? rates.split(";") : JSSCPort.getBaudRates();
        progRates.setList(rateList);
        if (firstSelect) {
          firstSelect = false;
          String port = prefs.get("programmer.port", null);
          if (port != null) {
            progPorts.setSelectedItem(port);
          }
          String rate = prefs.get("programmer.rate", null);
          if (rate != null) {
            progRates.setSelectedItem(rate);
          }
        } else {
          String port = prefs.get("programmer." + programmer + ".port", null);
          if (port != null) {
            progPorts.setSelectedItem(port);
          }
          String rate = prefs.get("programmer." + programmer + ".rate", null);
          if (rate != null) {
            progRates.setSelectedItem(rate);
          }
        }
        progPorts.addActionListener(ev2 -> {
          String label = (String) progMenu.getSelectedItem();
          String port = (String) progPorts.getSelectedItem();
          if (label != null && port != null) {
            prefs.put("programmer." + label + ".port", port);
          }
        });
        progRates.addActionListener(ev3 -> {
          String label = (String) progMenu.getSelectedItem();
          String rate = (String) progRates.getSelectedItem();
          if (label != null && rate != null) {
            prefs.put("programmer." + label + ".rate", rate);
          }
        });
      } else {
        progPorts.setList(new String[0]);
        progRates.setList(new String[0]);
        firstSelect = false;
      }
    });
    String selected = prefs != null ? prefs.get("programmer.name", "") : "Arduino TPI";
    progMenu.setSelectedItem(selected);
    Map<String, String> json = progList.get(selected);
    if (json != null) {
      setIcon(json.get("image"));
    }
    iconPanel.add(controls, BorderLayout.EAST);
    // Add dialog controls
    String[] buttons = new String[] {"Save", "Cancel"};
    JOptionPane optionPane = new JOptionPane(iconPanel, JOptionPane.PLAIN_MESSAGE, JOptionPane.YES_NO_OPTION, null, buttons, buttons[0]);
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
            if (prefs != null) {
              String sel = (String) progMenu.getSelectedItem();
              if (sel != null) {
                // Remember protocols and connection type for selected programmer
                Map<String, String> map = progList.get(sel);
                prefs.put("programmer.name", sel);
                prefs.put("programmer.programmer", map.get("programmer"));
                prefs.put("programmer.protocols",  map.get("protocols"));
                prefs.put("programmer.connection", map.get("connection"));
                // Remember programmer selection
                prefs.put("programmer.name", sel);
                // Remember port selection
                String port = (String) progPorts.getSelectedItem();
                prefs.put("programmer.port", port != null ? port : "");
                // Remember rate selection
                String rate = (String) progRates.getSelectedItem();
                prefs.put("programmer.rate", rate != null ? rate : "");
              }
            }
          }
          dispose();
        }
      }
    });
    pack();
    setResizable(false);
  }

  boolean wasPressed () {
    return !cancelled;
  }

  public static void main (String[] args) throws Exception {
    ProgrammerDialog dialog = new ProgrammerDialog(null);
    dialog.setLocation(2000, 200);
    dialog.setVisible(true);
    if (dialog.wasPressed()) {
      System.out.println("save");
    } else {
      System.out.println("cancel");
    }
  }
}
