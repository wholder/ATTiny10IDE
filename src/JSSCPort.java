import java.util.*;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.regex.Pattern;

import jssc.*;

/*
 * Encapsulates JSSC functionality into an easy to use class
 * See: https://code.google.com/p/java-simple-serial-connector/
 * And: https://github.com/scream3r/java-simple-serial-connector/releases
 *
 * Native drivers installed at path: ~/.jssc/mac_os_x/
 *    libjSSC-2.6_x86_64.jnilib
 *    libjSSC-2.8_x86.jnilib
 *    libjSSC-2.8_x86_64.jnilib
 *
 *  Author: Wayne Holder, 2015-2017 (first version 10/30/2015)
 */

public class JSSCPort implements SerialPortEventListener {
  private static final Map<String,Integer>  baudRates = new LinkedHashMap<>();
  private final ArrayBlockingQueue<Integer> queue = new ArrayBlockingQueue<>(1000);
  private String              portName;
  private int                 baudRate;
  private static final int    dataBits = 8;
  private static final int    stopBits = 1;
  private static final int    parity = 0;
  private int                 eventMasks;   // See: SerialPort.MASK_RXCHAR, MASK_TXEMPTY, MASK_CTS, MASK_DSR
  private static final int    flowCtrl = SerialPort.FLOWCONTROL_NONE;
  private SerialPort          serialPort;
  private boolean             hasListener;
  private final List<RXEvent> rxHandlers = new ArrayList<>();

  interface RXEvent {
    void rxChar (byte cc);
  }

  static {
    baudRates.put("110",    SerialPort.BAUDRATE_110);
    baudRates.put("300",    SerialPort.BAUDRATE_300);
    baudRates.put("600",    SerialPort.BAUDRATE_600);
    baudRates.put("1200",   SerialPort.BAUDRATE_1200);
    baudRates.put("2400",   2400);    // Note: constant missing in JSSC
    baudRates.put("4800",   SerialPort.BAUDRATE_4800);
    baudRates.put("9600",   SerialPort.BAUDRATE_9600);
    baudRates.put("14400",  SerialPort.BAUDRATE_14400);
    baudRates.put("19200",  SerialPort.BAUDRATE_19200);
    baudRates.put("38400",  SerialPort.BAUDRATE_38400);
    baudRates.put("57600",  SerialPort.BAUDRATE_57600);
    baudRates.put("115200", SerialPort.BAUDRATE_115200);
    baudRates.put("128000", SerialPort.BAUDRATE_128000);
    baudRates.put("256000", SerialPort.BAUDRATE_256000);
  }

  JSSCPort () {
  }

  public boolean isOpen () {
    if (serialPort != null) {
      return serialPort.isOpened();
    }
    return false;
  }

  public void setPort (String port) {
    portName = port;
  }

  public void setRate (String rate) {
    baudRate = baudRates.get(rate);
  }

  static String[] getPortNames () {
    // Determine OS Type
    switch (SerialNativeInterface.getOsType()) {
      case SerialNativeInterface.OS_LINUX:
        return SerialPortList.getPortNames(Pattern.compile("(ttyS|ttyUSB|ttyACM|ttyAMA|rfcomm)[0-9]{1,3}"));
      case SerialNativeInterface.OS_MAC_OS_X:
        return SerialPortList.getPortNames( Pattern.compile("cu."));
      case SerialNativeInterface.OS_WINDOWS:
        return SerialPortList.getPortNames(Pattern.compile(""));
      default:
        return SerialPortList.getPortNames(Pattern.compile("tty.*"));
    }
  }

  static String[] getBaudRates () {
    List<String> rates = new ArrayList<>(baudRates.keySet());
    return rates.toArray(new String[0]);
  }

  public boolean open (RXEvent handler) throws SerialPortException {
    if (serialPort != null) {
      if (serialPort.isOpened()) {
        close();
      }
    }
    if (portName != null) {
      serialPort = new SerialPort(portName);
      serialPort.openPort();
      serialPort.setParams(baudRate, dataBits, stopBits, parity, false, false);  // baud, 8 bits, 1 stop bit, no parity
      serialPort.setEventsMask(eventMasks);
      serialPort.setFlowControlMode(flowCtrl);
      serialPort.addEventListener(JSSCPort.this);
      setRXHandler(handler);
      hasListener = true;
      return true;
    }
    return false;
  }

  void close () {
    if (serialPort != null && serialPort.isOpened()) {
      try {
        synchronized (this) {
          rxHandlers.clear();
        }
        if (hasListener) {
          serialPort.removeEventListener();
          hasListener = false;
        }
        serialPort.closePort();
        serialPort = null;
      } catch (SerialPortException ex) {
        ex.printStackTrace();
      }
    }
  }

  public void serialEvent (SerialPortEvent se) {
    try {
      if (se.getEventType() == SerialPortEvent.RXCHAR) {
        int rxCount = se.getEventValue();
        byte[] inChars = serialPort.readBytes(rxCount);
        if (rxHandlers.size() > 0) {
          for (byte cc : inChars) {
            for (RXEvent handler : rxHandlers) {
              handler.rxChar(cc);
            }
          }
        } else {
          for (byte cc : inChars) {
            if (queue.remainingCapacity() > 0) {
              queue.add((int) cc);
            }
          }
        }
      }
    } catch (Exception ex) {
      ex.printStackTrace();
    }
  }

  private void setRXHandler (RXEvent handler) {
    synchronized (this) {
      rxHandlers.add(handler);
    }
  }

  void sendString (String data) throws SerialPortException {
    serialPort.writeString(data);
  }
}
