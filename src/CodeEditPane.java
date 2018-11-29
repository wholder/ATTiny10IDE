import cppsyntaxpane.DefaultSyntaxKit;
import cppsyntaxpane.lexers.CppLexer;

import javax.swing.*;
import javax.swing.text.Document;
import javax.swing.text.JTextComponent;
import javax.swing.text.PlainDocument;
import java.awt.*;
import java.util.EventListener;
import java.util.prefs.Preferences;

/**
 *  Code Editing Text Pane with CPP formatting and Search Features
 *  Author: Wayne Holder, 2017
 *  License: MIT (https://opensource.org/licenses/MIT)
 */

public class CodeEditPane extends JPanel {
  private JEditorPane         codePane;
  private CodeChangeListener  codeChangeListener;
  private DefaultSyntaxKit    synKit;
  private Preferences         prefs;


  CodeEditPane (Preferences prefs) {
    this.prefs = prefs;
    setLayout(new BorderLayout());
    synKit = new DefaultSyntaxKit(new CppLexer());
    codePane = new JEditorPane();
    synKit.addComponents(codePane);
    JScrollPane scroll = new JScrollPane(codePane);
    add(scroll, BorderLayout.CENTER);
    doLayout();
    // Note: must call setContentType(), setFont() after doLayout() or no line numbers and small font
    codePane.setContentType("text/cpp");
    boolean windows = System.getProperty("os.name").toLowerCase().contains("win");
    codePane.setFont(new Font(windows ? "Consolas" : "Menlo", Font.PLAIN, 12));
    codePane.setEditable(true);
  }

  JMenu getEditMenu () {
      return synKit.getEditMenu(codePane);
  }

  void setText (String text) {
    codePane.setText(text);
    codePane.setCaretPosition(0);
  }

  void setPosition (int line, int column) {
    int p = getDocumentPosition(line - 1, column);
    codePane.setCaretPosition(p);

  }

  private int getDocumentPosition (int line, int column) {
    int lineHeight = codePane.getFontMetrics(codePane.getFont()).getHeight();
    int charWidth = codePane.getFontMetrics(codePane.getFont()).charWidth('m');
    int y = line * lineHeight;
    int x = column * charWidth;
    Point pt = new Point(x, y);
    return codePane.viewToModel(pt);
  }


  JMenu getTabSizeMenu () {
    JMenu tabs = new JMenu("Tab Size");
    ButtonGroup group = new ButtonGroup();
    int tabSize = prefs.getInt("text.tabPane", 4);
    setTabSize(tabSize);
    for (int ii = 2; ii <= 8; ii += 2) {
      JRadioButtonMenuItem item = new JRadioButtonMenuItem("" + ii, tabSize == ii);
      item.addActionListener(e -> {
        int newSize = Integer.parseInt(item.getText());
        setTabSize(newSize);
        prefs.putInt("text.tabPane", newSize);
      });
      group.add(item);
      tabs.add(item);
    }
    return tabs;
  }

  void setTabSize (int tabSize) {
    Document doc = codePane.getDocument();
    doc.putProperty(PlainDocument.tabSizeAttribute, tabSize);
    codePane.updateUI();
  }

  JMenu getFontSizeMenu () {
    JMenu tabsItem = new JMenu("Font Size");
    ButtonGroup group = new ButtonGroup();
    int fontSize = prefs.getInt("text.fontsize", 12);
    setFontSize(fontSize);
    for (int ii : new int[] {10, 12, 14, 16, 18}) {
      JRadioButtonMenuItem item = new JRadioButtonMenuItem("" + ii, fontSize == ii);
      item.addActionListener(e -> {
        int newSize = Integer.parseInt(item.getText());
        setFontSize(newSize);
        prefs.putInt("text.fontsize", newSize);
      });
      group.add(item);
      tabsItem.add(item);
    }
    return tabsItem;
  }

  void setFontSize (int points) {
    boolean windows = System.getProperty("os.name").toLowerCase().contains("win");
    codePane.setFont(new Font(windows ? "Consolas" : "Menlo", Font.PLAIN, points));
    codePane.updateUI();
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
}
