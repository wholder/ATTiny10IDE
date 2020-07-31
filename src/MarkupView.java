import com.github.rjeschke.txtmark.Processor;

import javax.imageio.ImageIO;
import javax.swing.*;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import javax.swing.text.*;
import javax.swing.text.html.HTML;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.html.ImageView;
import javax.swing.text.html.StyleSheet;
import java.awt.*;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.image.BufferedImage;
import java.io.*;
import java.net.URI;
import java.net.URL;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Base64;
import java.util.prefs.Preferences;

class MarkupView extends JPanel {
  private final JEditorPane           jEditorPane;
  private final ArrayList<StackItem>  stack = new ArrayList<>();
  private final String                codeFont;
  private String                      basePath, currentPage;

  {
    String os = System.getProperty("os.name").toLowerCase();
    if (os.contains("win")) {
      codeFont = "Consolas";
    } else if (os.contains("mac")) {
      codeFont ="Menlo";
    } else {
      codeFont ="Courier";
    }
  }

  private static class StackItem {
    private final String  location;
    private final Point   position;

    private StackItem (String location, Point position) {
      this.location = location;
      this.position = position;
    }
  }

  class MyImageView extends ImageView {
    private final boolean flatten;
    private String        loc;
    private Image         img;

    private MyImageView (Element elem) {
      super(elem);
      try {
        loc = URLDecoder.decode((String) elem.getAttributes().getAttribute(HTML.Attribute.SRC), "UTF-8");
      } catch (UnsupportedEncodingException ex) {
        ex.printStackTrace();
      }
      String tmp = (String) elem.getAttributes().getAttribute("flatten");
      flatten = "1".equals(tmp) ||  "true".equals(tmp);
    }

    @Override
    public URL getImageURL () {
      return getClass().getResource(basePath + loc);
    }

    @Override
    public Image getImage () {
      if (img != null) {
        return img;
      } else {
        if (loc.startsWith("data:")) {
          int idx = loc.indexOf(",");
          String b64 = loc.substring(idx + 1);
          //b64 = b64.replace(' ', '+');          // Note: not needed if we URL Encode Base64
          try {
            ByteArrayInputStream buf = new ByteArrayInputStream(Base64.getDecoder().decode(b64));
            BufferedImage baseImg = ImageIO.read(buf);
            if (flatten) {
              // Create non-transparent image to remove black border on embedded images
              BufferedImage bufImg = new BufferedImage(baseImg.getWidth(), baseImg.getHeight(), BufferedImage.TYPE_INT_RGB);
              Graphics2D g2 = bufImg.createGraphics();
              g2.setColor(Color.white);
              g2.fillRect(0, 0, bufImg.getWidth(), bufImg.getHeight());
              g2.drawImage(baseImg, 0, 0, null);
              g2.dispose();
              return img = bufImg;
            }
            return img = baseImg;
          } catch (Throwable ex) {
            ex.printStackTrace();
          }
        }
      }
      return img = super.getImage();
    }
  }

  class MyViewFactory implements ViewFactory {
    ViewFactory view;

    private MyViewFactory (ViewFactory view) {
      this.view = view;
    }

    public View create (Element elem) {
      AttributeSet attrs = elem.getAttributes();
      Object elementName = attrs.getAttribute(AbstractDocument.ElementNameAttribute);
      Object obj = (elementName != null) ? null : attrs.getAttribute(StyleConstants.NameAttribute);
      if (obj instanceof HTML.Tag && obj == HTML.Tag.IMG) {
        return new MyImageView(elem);
      }
      return view.create(elem);
    }
  }

  class MyEditorKit extends HTMLEditorKit {
    @Override
    public ViewFactory getViewFactory () {
      return new MyViewFactory(super.getViewFactory());
    }
  }

  MarkupView (String loc) {
    this();
    loadMarkup(loc);
  }

  MarkupView () {
    setLayout(new BorderLayout());
    jEditorPane = new JEditorPane();
    JScrollPane scrollPane = new JScrollPane(jEditorPane);
    JButton back = new JButton("BACK");
    jEditorPane.addHyperlinkListener(new HyperlinkListener() {
      private String tooltip;
      @Override public void hyperlinkUpdate(HyperlinkEvent ev) {
        JEditorPane editor = (JEditorPane) ev.getSource();
        if (ev.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
          String link = ev.getDescription();
          if (link.startsWith("http://") || link.startsWith("https://")) {
            if (Desktop.isDesktopSupported()) {
              try {
                Desktop.getDesktop().browse(new URI(link));
              } catch (Exception ex) {
                ex.printStackTrace();
              }
            }
          } else {
            Point scrollPosition = scrollPane.getViewport().getViewPosition();
            stack.add(new StackItem(currentPage, scrollPosition));
            loadMarkup(link);
            back.setVisible(stack.size() > 0);
          }
        } else if (ev.getEventType() == HyperlinkEvent.EventType.ENTERED) {
          tooltip = editor.getToolTipText();
          String text = ev.getDescription();
          editor.setToolTipText(text);
        } else if (ev.getEventType() == HyperlinkEvent.EventType.EXITED) {
          editor.setToolTipText(tooltip);
        }
      }
    });
    jEditorPane.setEditable(false);
    scrollPane.setBorder(BorderFactory.createEmptyBorder(5, 10, 5, 10));
    add(scrollPane, BorderLayout.CENTER);
    back.addActionListener(e -> {
      if (stack.size() > 0) {
        StackItem item = stack.remove(stack.size() - 1);
        loadMarkup(item.location);
        back.setVisible(stack.size() > 0);
        SwingUtilities.invokeLater(() -> scrollPane.getViewport().setViewPosition(item.position));
      }
    });
    add(back, BorderLayout.NORTH);
    back.setVisible(false);
    HTMLEditorKit kit = new MyEditorKit();
    jEditorPane.setEditorKit(kit);
    // Setup some basic markdown styles
    StyleSheet styleSheet = kit.getStyleSheet();
    styleSheet.addRule("body {color:#000; font-family: Bookman, Arial, Helvetica; margin: 4px;}");
    styleSheet.addRule("h1 {font-size: 24px; font-weight: 500;}");
    styleSheet.addRule("h2 {font-size: 20px; font-weight: 500;}");
    styleSheet.addRule("h3 {font-size: 16px; font-weight: 500;}");
    styleSheet.addRule("h4 {font-size: 14px; font-weight: 500;}");
    styleSheet.addRule("h5 {font-size: 12px; font-weight: 500;}");
    styleSheet.addRule("h6 {font-size: 10px; font-weight: 500;}");
    styleSheet.addRule("pre {margin-left: 0.5cm;}");
    styleSheet.addRule("ol {margin-left: 1cm;}");
    styleSheet.addRule("ol li {font-size: 12px; margin-top: 3px; margin-bottom: 3px;}");
    styleSheet.addRule("ul li {font-size: 12px; margin-top: 3px; margin-bottom: 3px;}");
    styleSheet.addRule("code {font-family: " + codeFont + "; font-size: 12px; margin-bottom: 3px;}");
    styleSheet.addRule("p {font-size: 12px; margin-top: 5px; margin-bottom: 5px;}");
  }

  public void setText (String markup) {
    String html = Processor.process(markup);
    jEditorPane.setText(html);
  }

  public void loadMarkup (String loc) {
    if (loc != null) {
      if (basePath == null) {
        int idx = loc.lastIndexOf("/");
        if (idx >= 0) {
          basePath = loc.substring(0, idx + 1);
          loc = loc.substring(idx + 1);
        } else {
          basePath = "";
        }
      }
      try {
        setText(new String(getResource(basePath + loc)));
        currentPage = loc;
        jEditorPane.setCaretPosition(0);
      } catch (Exception ex) {
        ex.printStackTrace();
      }
    }
  }

  private byte[] getResource (String file) throws IOException {
    InputStream fis = MarkupView.class.getClassLoader().getResourceAsStream(file);
    if (fis != null) {
      byte[] data = new byte[fis.available()];
      fis.read(data);
      fis.close();
      return data;
    }
    throw new IllegalStateException("MarkupView.getResource() " + file + " not found");
  }

  /*
   *  Test code for MarkupView Pane
   */
  public static void main (String[] args) {
    Preferences prefs = Preferences.userRoot().node(MarkupView.class.getName());
    JFrame frame = new JFrame();
    MarkupView mView = new MarkupView("documentation/index.md");
    frame.add(mView, BorderLayout.CENTER);
    frame.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
    frame.setSize(prefs.getInt("window.width", 800), prefs.getInt("window.height", 900));
    frame.setLocation(prefs.getInt("window.x", 10), prefs.getInt("window.y", 10));
    // Add window close handler
    frame.addWindowListener(new WindowAdapter() {
      public void windowClosing (WindowEvent ev) {
        System.exit(0);
      }
    });
    // Track window resize/move events and save in prefs
    frame.addComponentListener(new ComponentAdapter() {
      public void componentMoved (ComponentEvent ev) {
        Rectangle bounds = ev.getComponent().getBounds();
        prefs.putInt("window.x", bounds.x);
        prefs.putInt("window.y", bounds.y);
      }

      public void componentResized (ComponentEvent ev) {
        Rectangle bounds = ev.getComponent().getBounds();
        prefs.putInt("window.width", bounds.width);
        prefs.putInt("window.height", bounds.height);
      }
    });
    frame.setVisible(true);
  }
}
