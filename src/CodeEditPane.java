import jsyntaxpane.DefaultSyntaxKit;

import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.text.Document;
import javax.swing.text.PlainDocument;
import javax.swing.undo.CannotRedoException;
import javax.swing.undo.CannotUndoException;
import javax.swing.undo.UndoManager;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.InputEvent;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.util.EventListener;
import java.util.prefs.Preferences;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 *  Code Editing Text Pane with CPP formatting and Search Features
 *  Author: Wayne Holder, 2017
 *  License: MIT (https://opensource.org/licenses/MIT)
 */

public class CodeEditPane extends JPanel implements DocumentListener {
  private static int        cmdMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
  private static KeyStroke  UNDO_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_Z, cmdMask) ;
  private static KeyStroke  REDO_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_Z, cmdMask + InputEvent.SHIFT_MASK) ;
  private static KeyStroke  FIND_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_F, cmdMask) ;
  private static KeyStroke  FIND_AGAIN_KEY = KeyStroke.getKeyStroke(KeyEvent.VK_G, cmdMask) ;
  private Preferences       prefs;
  private JEditorPane       codePane;
  private JPanel            searchBar;
  private JTextField        searchField;
  private JCheckBox         matchCase, fullWords;
  private Document          doc;
  private UndoManager       undo = new UndoManager();
  private int               tabSize;
  private CodeChangeListener codeChangeListener;

  CodeEditPane (Preferences prefs) {
    this.prefs = prefs;
    setLayout(new BorderLayout());
    tabSize = prefs.getInt("text.tabPane", 4);
    DefaultSyntaxKit.initKit();
    codePane = new JEditorPane();
    codePane.setFont(new Font("Courier", Font.PLAIN, 14));
    codePane.setEditable(true);
    doc = codePane.getDocument();
    doc.putProperty(PlainDocument.tabSizeAttribute, tabSize);
    JScrollPane scroll = new JScrollPane(codePane);
    add(scroll, BorderLayout.CENTER);
    // Build Search and Replace Box
    searchBar = new JPanel(new GridBagLayout());
    searchBar.setVisible(false);
    searchBar.setBorder(BorderFactory.createEmptyBorder(2, 1, 0, 1));
    searchBar.setBackground(Color.lightGray);
    int idx = 0;
    // Add Magnifiying Glass Icon
    JLabel mGlass = new JLabel(new ImageIcon(getClass().getResource("images/find-24x24.png")));
    searchBar.add(mGlass, getGbc(idx++, .01));
    // Add Text Field for search text
    searchField = new JTextField(20);
    searchBar.add(searchField, getGbc(idx++, .01));
    // Add key listener for Enter key
    searchField.addKeyListener(new KeyAdapter() {
      @Override
      public void keyPressed(KeyEvent ev) {
        if(ev.getKeyCode() == KeyEvent.VK_ENTER) {
          search(searchField.getText());
        }
      }
    });
    // Add "Search" Button
    JButton findButton = new JButton("Find");
    findButton.addActionListener(e -> search(searchField.getText()));
    //findButton.setPreferredSize(new Dimension(60, 10));
    searchBar.add(findButton, getGbc(idx++, .001));
    // Add "Match Case" Checkbox
    matchCase = new JCheckBox("Match Case", prefs.getBoolean("matchcase", false));
    matchCase.addActionListener(e -> prefs.putBoolean("matchcase", matchCase.isSelected()));
    searchBar.add(matchCase, getGbc(idx++, .001));
    // Add "Words" Checkbox
    fullWords = new JCheckBox("Words", prefs.getBoolean("fullwords", false));
    fullWords.addActionListener(e -> prefs.putBoolean("fullwords", fullWords.isSelected()));
    searchBar.add(fullWords, getGbc(idx++, .001));
    // Add Blank area
    searchBar.add(new JLabel(""), getGbc(idx++, 1.0));
    // Add close control for Search Bar
    JButton closeBar = new JButton("x");
    closeBar.setToolTipText("Close Search Bar");
    closeBar.setPreferredSize(new Dimension(22, 22));
    closeBar.addActionListener(e -> searchBar.setVisible(false));
    searchBar.add(closeBar, getGbc(idx++, .001));
    add(searchBar, BorderLayout.NORTH);
    doLayout();
    // Note: must call setContentType() after doLayout() or line numbers will not show
    codePane.setContentType("text/cpp");
    // Setup Undo and Redo Handlers
    doc.addUndoableEditListener(evt -> undo.addEdit(evt.getEdit()));
    // Create an undo action and add it to the text component
    codePane.getActionMap().put("Undo", new AbstractAction("Undo") {
      public void actionPerformed (ActionEvent evt) {
        try {
          if (undo.canUndo()) {
            undo.undo();
          }
        } catch (CannotUndoException ex) {
          ex.printStackTrace();
        }
      }
    });
    // Bind the undo action to ctl-Z (or command-Z on mac)
    codePane.getInputMap().put(UNDO_KEY, "Undo");
    // Create a redo action and add it to the text component
    codePane.getActionMap().put("Redo", new AbstractAction("Redo") {
      public void actionPerformed (ActionEvent evt) {
        try {
          if (undo.canRedo()) {
            undo.redo();
          }
        } catch (CannotRedoException ex) {
          ex.printStackTrace();
        }
      }
    });
    // Bind the redo action to ctl-Y (or command-Y on mac)
    codePane.getInputMap().put(REDO_KEY, "Redo");
    codePane.getDocument().addDocumentListener(this);
  }

  /**
   * Builds an "Edit" menu for this Component that an implementing JFrame can use to operate it.
   * @return JMenu object that controls this Component
   */
  JMenu getEditMenu () {
    JMenu editMenu = new JMenu("Edit");
    JMenu tabsItem = new JMenu("Tabs");
    ButtonGroup group = new ButtonGroup();
    for (int ii = 2; ii <= 8; ii += 2) {
      JRadioButtonMenuItem item = new JRadioButtonMenuItem("" + ii, tabSize == ii);
      item.addActionListener(e -> {
        tabSize = Integer.parseInt(item.getText());
        doc.putProperty(PlainDocument.tabSizeAttribute, tabSize);
        prefs.putInt("text.tabPane", tabSize);
      });
      group.add(item);
      tabsItem.add(item);
    }
    editMenu.add(tabsItem);
    // Add separator
    editMenu.addSeparator();
    // Add Undo Menu Item
    JMenuItem undoItem = new JMenuItem("Undo");
    // Note: Set accel keys for undo and redo  just so the codes show in the menus
    undoItem.setAccelerator(UNDO_KEY);
    editMenu.add(undoItem);
    // Add Redo Menu Item
    JMenuItem redoItem = new JMenuItem("Redo");
    redoItem.setAccelerator(REDO_KEY);
    editMenu.add(redoItem);
    // Add separator
    editMenu.addSeparator();
    // Add "Find" Menu Item
    JMenuItem findItem = new JMenuItem("Find");
    findItem.setAccelerator(FIND_KEY);
    findItem.addActionListener(e -> {
      int start = codePane.getSelectionStart();
      int end = codePane.getSelectionEnd();
      if ((end - start) > 0) {
        searchField.setText(codePane.getText().substring(start, end));
      }
      searchBar.setVisible(true);
    });
    editMenu.add(findItem);
    // Add "Find Again" Menu Item
    JMenuItem findAgain = new JMenuItem("Find Again");
    findAgain.setAccelerator(FIND_AGAIN_KEY);
    findAgain.addActionListener(e -> {
      if (searchBar.isVisible() && searchField.getText().length() > 0) {
        search(searchField.getText());
      }
    });
    editMenu.add(findAgain);
    return editMenu;
  }

  /*
   * Find the next occurrence of the string entered in the search field.  The setting of the "case sensitive"
   * box will determine if the search requires the same capitalization or not.
   */
  private void search (String searchValue) {
    String editorText = codePane.getText();
    // If the search should be case insensitive, change the editing text and search string to all lower case
    if (!matchCase.isSelected()) {
      editorText = editorText.toLowerCase();
      searchValue = searchValue.toLowerCase();
    }
    // Find the next occurrence of the text
    int start;
    if (fullWords.isSelected()) {
      Pattern pat = Pattern.compile("\\b" + searchValue + "\\b");
      Matcher mat = pat.matcher(editorText);
      start = mat.find(codePane.getSelectionEnd()) ? mat.start() : -1;
    } else {
      start = editorText.indexOf(searchValue, codePane.getSelectionEnd());
    }
    // If the string was found, move the selection so that the found string is highlighted.
    if (start >= 0) {
      codePane.setCaretPosition(start);
      codePane.moveCaretPosition(start + searchValue.length());
      codePane.getCaret().setSelectionVisible(true);
    } else {
      // Try resuming the search from the start of the text
      if (fullWords.isSelected()) {
        Pattern pat = Pattern.compile("\\b" + searchValue + "\\b");
        Matcher mat = pat.matcher(editorText);
        start = mat.find(0) ? mat.start() : -1;
      } else {
        start = editorText.indexOf(searchValue, codePane.getSelectionEnd());
      }
      if (start >= 0) {
        codePane.setCaretPosition(start);
        codePane.moveCaretPosition(start + searchValue.length());
        codePane.getCaret().setSelectionVisible(true);
      }
    }
  }

  private GridBagConstraints getGbc (int x, double weight) {
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.gridx = x;
    gbc.weightx = weight;
    gbc.anchor = GridBagConstraints.WEST;
    return gbc;
  }

  void setText (String text) {
    codePane.setText(text);
    codePane.setCaretPosition(0);
  }

  String getText () {
    return codePane.getText();
  }

  interface CodeChangeListener extends EventListener {
    void codeChanged ();
  }

  void setCodeChangeListener (CodeChangeListener codeChangeListener) {
    this.codeChangeListener = codeChangeListener;
  }

  private void codeChanged () {
    if (codeChangeListener != null) {
      codeChangeListener.codeChanged();
    }
  }

  // Implement DocumentListener interface

  public void insertUpdate (DocumentEvent ev) {
    codeChanged();
  }

  public void removeUpdate (DocumentEvent ev) {
    codeChanged();
  }

  public void changedUpdate (DocumentEvent ev) {
    codeChanged();
  }
}
