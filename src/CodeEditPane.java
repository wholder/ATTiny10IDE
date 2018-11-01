import cppsyntaxpane.DefaultSyntaxKit;
import cppsyntaxpane.lexers.CppLexer;

import javax.swing.*;
import javax.swing.text.Document;
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


  CodeEditPane (Preferences prefs) {
    setLayout(new BorderLayout());
    int tabSize = prefs.getInt("text.tabPane", 4);
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
    Document doc = codePane.getDocument();
    doc.putProperty(PlainDocument.tabSizeAttribute, tabSize);
  }

  JMenu getEditMenu () {
      return synKit.getEditMenu(codePane);
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
}
