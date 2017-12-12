import java.io.*;
import java.util.*;
import java.math.*;

/**
 *  Converts infix expression to postfix expression using this algorithm:
 *    1. Initialize an empty stack and empty output List.
 *    2. Read the infix expression from left to right, one token at a time.
 *    3. If the token is an operand, append it to the output List.
 *    4. If the token is an operator, pop operators until you reach an opening
 *       parenthesis, an operator of lower precedence, or a right associative symbol
 *       of equal precedence. Push the operator onto the stack.
 *    5. If the token is an opening parenthesis, push it onto the stack.
 *    6. If the token is a closing parenthesis, pop all operators until you reach
 *       an opening parenthesis and append them to the output List.
 *    7. If the end of the input is found, pop all operators and append them
 *       to the output List.
 *  Limitations:
 *    1. Negative numeric constants, such as -23, require leading space to delimit.
 *
 *  Author: Wayne Holder, 2017
 *  License: MIT (https://opensource.org/licenses/MIT)
 */

public class ExpressionParser {
  private static final Null   NULL = new Null();
  private static BigInteger   INT256 = new BigInteger("256");
  private static BigInteger   INT255 = new BigInteger("255");
  private static Map<String, Integer> ops = new HashMap<>();
  private static Map<String, Integer> funcs = new HashMap<>();

  static {
    ops.put("|",  1); // lowest
    ops.put("||", 1);
    ops.put("&",  1);
    ops.put("&&", 1);
    ops.put("^",  1);

    ops.put("S&", 2); // shortcut operators (used internally)
    ops.put("S|", 2);

    ops.put("==", 3);
    ops.put("!=", 3);

    ops.put("<",  4);
    ops.put("<=", 4);
    ops.put(">",  4);
    ops.put(">=", 4);

    ops.put("<<", 5); // signed left shift
    ops.put(">>", 5); // signed right shift
    ops.put(">>>", 5);

    ops.put("-",  6);
    ops.put("+",  6);

    ops.put("/",  7);
    ops.put("*",  7);
    ops.put("%",  7);

    ops.put("!",  8);

    ops.put("(",  9);
    ops.put(")",  9);

    ops.put("$$", 10); // highest
    // Define functions
    funcs.put("max", 0);
    funcs.put("min", 0);
    funcs.put("high", 0); // Select upper byte of 16 bit word
    funcs.put("low", 0);  // Select lower byte of 16 bit word
  }

  public static class Token {
    private static final int VAR = 0;
    private static final int VAL = 1;
    private static final int STR = 2;
    private static final int OP  = 3;
    private static final int FNC = 4;
    private static final int CMA = 5;
    private static final int EXP = 6;
    private String          val;
    private int             shortcutId = -1;
    private int             type;
    private int             prec;
    private boolean         isShortcut;

    private Token (String val, int type) {
      this.val = val;
      this.type = type;
      if (type == OP) {
        prec = ops.get(val);
      } else if (type == FNC) {
        prec = ops.get("$$");
        isShortcut = val.equals("SHORT_T") | val.equals("SHORT_F");
      }
    }

    private Token (String val, int type, int shortcutId) {
      this(val, type);
      this.shortcutId = shortcutId;
    }

    public String toString () {
      return shortcutId >= 0 ? val + ":" + shortcutId : val;
    }
  }

  /**
   * Parse infix into List in postfix order. Note: Strings like "TEST" are treated as
   * variable names, and ones with surrounding ' marks, such as "'TEST'" are treated as
   * string literals.
   * @param in infix expression
   * @return Token[] array postfix expression
   */
  public static Token[] parse (String in) {
    try {
      Token[] expr = tokenize(in);
      List<Token> out = new ArrayList<>();
      LinkedList<Token> stack = new LinkedList<>();
      for (Token tok : expr) {
        Token top;
        switch (tok.type) {
        case Token.VAR:
        case Token.VAL:
        case Token.STR:
          out.add(tok);
          break;
        case Token.OP:
        case Token.FNC:
          switch (tok.val) {
          case ")":
            while (!stack.isEmpty() && !(top = stack.getLast()).val.equals("(")) {
              stack.removeLast();
              out.add(top);
            }
            stack.removeLast();
            if (!stack.isEmpty()) {
              top = stack.getLast();
              if (top.isShortcut || top.type == Token.FNC) {
                stack.removeLast();
                out.add(top);
              }
            }
            break;
          case "(":
            stack.add(tok);
            break;
          default:
            while (!stack.isEmpty() && !(top = stack.getLast()).val.equals("(") && top.prec > tok.prec) {
              stack.removeLast();
              if (!top.val.equals("(") && !top.val.equals(")")) {
                out.add(top);
              }
            }
            stack.add(tok);
            break;
          }
          break;
        case Token.CMA:
          while (!stack.isEmpty() && !(top = stack.getLast()).val.equals("(")) {
            stack.removeLast();
            out.add(top);
          }
          break;
        }
      }
      while (!stack.isEmpty()) {
        Token rem = stack.removeLast();
        if (!rem.val.equals("(") && !rem.val.equals(")")) {
          out.add(rem);
        }
      }
      //System.out.println(tokensToString(toks));
      return out.toArray(new Token[0]);
    } catch (Exception ex) {
      throw new IllegalStateException("Error parsing: '" + in + "'", ex);
    }
  }

  /**
   * Tokenize String into a Token[] array
   * @param in expression to tokenize
   * @return Token[] array in postfix form
   */
  private static Token[] tokenize (String in) {
    int id = 0;
    in = condenseWhitespace(in) + ' ';
    List<Token> out = new ArrayList<>();
    out.add(new Token(in, Token.EXP));    // Save expression for stack trace display
    int len = in.length();
    int state = 0;
    StringBuilder acc = new StringBuilder();
    for (int ii = 0; ii < len; ii++) {
      char cc = in.charAt(ii);
      switch (state) {
      case 0: // waiting
        char c2 = ii < len - 1 ? in.charAt(ii + 1) : ' ';
        if (Character.isDigit(cc) || cc == '.' || (cc == '-' || cc == '+') && Character.isDigit(c2)) {
          acc.append(cc);
          state = 2;
        } else if (Character.isLetter(cc) || cc == '_') {
          acc.append(cc);
          state = 1;
        } else if (cc == '\'') {
          state = 3;
        } else {
          if ((cc == '<' || cc == '>') && c2 == '=' || cc == '=' && c2 == '=' || cc == '!' && c2 == '=') {
            out.add(new Token(in.substring(ii, ii + 2), Token.OP));
            ii++;
          } else if ((cc == '<' && c2 == '<') || (cc == '>' && c2 == '>')) {
            if (cc == '>' && ii < len - 2 && in.charAt(ii + 2) == '>') {
              out.add(new Token(in.substring(ii, ii + 3), Token.OP));
              ii += 2;
            } else {
              out.add(new Token(in.substring(ii, ii + 2), Token.OP));
              ii++;
            }
          } else if (cc == ',') {
            out.add(new Token(Character.toString(cc), Token.CMA));
          } else if (cc == '&') {
            if (c2 == '&') {
              out.add(new Token("S&", Token.OP, id));
              out.add(new Token("&&", Token.OP, id));
              id++;
              ii++;
            } else {
              out.add(new Token("&", Token.OP));
            }
          } else if (cc == '|') {
            if (c2 == '|') {
              out.add(new Token("S|", Token.OP, id));
              out.add(new Token("||", Token.OP, id));
              id++;
              ii++;
            } else {
              out.add(new Token("|", Token.OP));
            }
          } else if (!Character.isWhitespace(cc)) {
            out.add(new Token(Character.toString(cc), Token.OP));
          }
        }
        break;
      case 1: // variable
        if (Character.isLetter(cc)  ||  Character.isDigit(cc)  ||  cc == '.'  ||  cc == '_'  ||  cc == ':') {
          acc.append(cc);
        } else {
          String name = acc.toString();
          out.add(new Token(name, funcs.containsKey(name.toLowerCase()) ? Token.FNC : Token.VAR ));
          acc = new StringBuilder();
          state = 0;
          ii--;
        }
        break;
      case 2: // number
        if (Character.isDigit(cc) || cc == '.' ||  (acc.length() == 1 && cc == 'x')  ||
            (acc.length() >= 2 && acc.charAt(1) == 'x' && isHex(cc))) {
          acc.append(cc);
        } else {
          out.add(new Token(acc.toString(), Token.VAL));
          acc = new StringBuilder();
          state = 0;
          ii--;
        }
        break;
      case 3: // String
        if (cc != '\'') {
          acc.append(cc);
        } else {
          out.add(new Token(acc.toString(), Token.STR));
          acc = new StringBuilder();
          state = 0;
        }
        break;
      }
    }
    return out.toArray(new Token[0]);
  }

  private static class Null {
    @Override
    public boolean equals(Object obj) {
      return obj == null  ||  obj instanceof Null;
    }
  }

  private static boolean isHex (char cc) {
    return (cc >= 'a' && cc <= 'f') || (cc >= 'A' && cc <= 'F');
  }

  /**
   * Reformat String to reduce all whitespace to a single space
   * @param text Input text
   * @return Reformatted output
   */
  public static String condenseWhitespace (String text) {
    StringTokenizer tok = new StringTokenizer(text);
    StringBuilder buf = new StringBuilder();
    while (tok.hasMoreTokens()) {
      String line = tok.nextToken();
      buf.append(line);
      buf.append(' ');
    }
    return buf.toString().trim();
  }

  /**
   * Convert String[] into a String where each item is separated by a common String separator
   * @param tokens Token array of values that support toString() call
   */
  private static String tokensToString (Token[] tokens) {
    StringBuilder buf = new StringBuilder();
    boolean first = true;
    for (Object obj : tokens) {
      if (!first)
        buf.append(", ");
      first = false;
      buf.append(obj.toString());
    }
    return buf.toString();
  }

  /**
   * Evaluate the postfix expression in Token[] array expr using the variable
   * values provided in the vals Map and return the result.
   * @param expr Token[] array representing a postfix expression
   * @param vals Map that supplies values for all expression variables
   * @return Object containing result (Boolean or BigInteger)
   */
  public static Object eval (Token[] expr, Map vals) {
    try {
      LinkedList<Object> stack = new LinkedList<>();
      int shortcutId = -1;
      for (Token tok : expr) {
        if (shortcutId >= 0) {
          if (shortcutId == tok.shortcutId) {
            shortcutId = -1;
          }
          continue;
        }
        switch (tok.type) {
        case Token.EXP:
          // Ignore expression token (used for diagnostic purposes)
          break;
        case Token.VAL:
          if (tok.val.startsWith("0x")) {
            String tmp = tok.val.substring(2);
            stack.add(new BigInteger(tmp, 16));
          } else {
            stack.add(new BigInteger(tok.val));
          }
          break;
        case Token.STR:
          stack.add(tok.val);
          break;
        case Token.VAR:
          switch (tok.val) {
          case "true":
            stack.add(Boolean.TRUE);
            break;
          case "false":
            stack.add(Boolean.FALSE);
            break;
          case "null":
            stack.add(NULL);
            break;
          default:
            Object val = vals.get(tok.val);
            if (val instanceof Number) {
              stack.add(new BigInteger(val.toString()));
            } else if (val == null) {
              stack.add(NULL);
            } else {
              stack.add(val);
            }
            break;
          }
          break;
        case Token.OP:
          String op = tok.val;
          if (op.equals("!")) {
            Object arg = stack.removeLast();
            if (arg instanceof Boolean)
              stack.add(!(Boolean) arg ? Boolean.TRUE : Boolean.FALSE);
            else
              stack.add(((BigInteger) arg).not());
          } else {
            // Check for shortcut evaluation
            Object stkTop = stack.getLast();
            if (op.equals("S&")) {
              if (!((Boolean) stkTop)) {
                shortcutId = tok.shortcutId;
              }
              continue;
            } else if (op.equals("S|")) {
              if (((Boolean)stkTop)) {
                shortcutId = tok.shortcutId;
              }
              continue;
            }
            Object rArg = stack.removeLast();
            Object lArg = stack.removeLast();
            switch (op) {
            case "<":
            case "<=":
            case ">":
            case ">=":
              if (lArg instanceof Comparable && rArg instanceof Comparable) {
                int comp = ((Comparable) lArg).compareTo(rArg);
                switch (op) {
                case "<":
                  stack.add((comp < 0) ? Boolean.TRUE : Boolean.FALSE);
                  break;
                case "<=":
                  stack.add((comp <= 0) ? Boolean.TRUE : Boolean.FALSE);
                  break;
                case ">":
                  stack.add((comp > 0) ? Boolean.TRUE : Boolean.FALSE);
                  break;
                case ">=":
                  stack.add((comp >= 0) ? Boolean.TRUE : Boolean.FALSE);
                  break;
                default:
                  throw new IllegalArgumentException("Parser.eval() Unknown comparison " + op);
                }
              } else {
                throw new IllegalArgumentException("Parser.eval() args are not Comparable objects");
              }
              break;
            case "+":
              if (lArg instanceof BigInteger) {
                stack.add(((BigInteger) lArg).add((BigInteger) rArg));
              } else if (lArg instanceof String) {
                stack.add(((String) lArg).concat((String) rArg));
              } else {
                throw new IllegalArgumentException("Parser.eval() args are not compatible for '+' operation");
              }
              break;
            case "-":
              stack.add(((BigInteger) lArg).subtract((BigInteger) rArg));
              break;
            case "*":
              stack.add(((BigInteger) lArg).multiply((BigInteger) rArg));
              break;
            case "/":
              stack.add(((BigInteger) lArg).divide((BigInteger) rArg));
              break;
            case "<<":
              stack.add(((BigInteger) lArg).shiftLeft(((BigInteger) rArg).intValue()));
              break;
            case ">>":
              int shift = ((BigInteger) rArg).intValue();
              BigInteger divisor = BigInteger.ONE.shiftLeft((shift));
              stack.add(((BigInteger) lArg).divide(divisor));
              break;
            case ">>>":
              stack.add(((BigInteger) lArg).shiftRight(((BigInteger) rArg).intValue()));
              break;
            case "%":
              stack.add(((BigInteger) lArg).mod(((BigInteger) rArg)));
              break;
            case "&":
            case "&&":
              if (lArg instanceof Boolean && rArg instanceof Boolean) {
                stack.add((Boolean) lArg & (Boolean) rArg ? Boolean.TRUE : Boolean.FALSE);
              } else if (lArg instanceof BigInteger && rArg instanceof BigInteger) {
                stack.add(((BigInteger) lArg).and(((BigInteger) rArg)));
              }
              break;
            case "|":
            case "||":
              if (lArg instanceof Boolean && rArg instanceof Boolean) {
                stack.add((Boolean) lArg | (Boolean) rArg ? Boolean.TRUE : Boolean.FALSE);
              } else if (lArg instanceof BigInteger && rArg instanceof BigInteger) {
                stack.add(((BigInteger) lArg).or(((BigInteger) rArg)));
              }
              break;
            case "^":
              if (lArg instanceof Boolean && rArg instanceof Boolean) {
                stack.add((Boolean) lArg ^ (Boolean) rArg ? Boolean.TRUE : Boolean.FALSE);
              } else if (lArg instanceof BigInteger && rArg instanceof BigInteger) {
                stack.add(((BigInteger) lArg).xor(((BigInteger) rArg)));
              }
              break;
            case "==":
              stack.add(lArg.equals(rArg) ? Boolean.TRUE : Boolean.FALSE);
              break;
            case "!=":
              stack.add(!lArg.equals(rArg) ? Boolean.TRUE : Boolean.FALSE);
              break;
            default:
              throw new IllegalArgumentException("Parser.eval() Unknown operator " + op);
            }
          }
          break;
        case Token.FNC:
          String func = tok.val;
          Object arg = stack.size() > 0 ? stack.removeLast() : null;
          if ("max".equalsIgnoreCase(func)) {
            Object arg2 = stack.removeLast();   // leftmost arg
            if (!(arg instanceof BigInteger) || !(arg2 instanceof BigInteger))
              throw new IllegalStateException("ExpressionParser.eval() both args not BigInteger max(" + arg2 + ' ' + arg + ')');
            arg = ((BigInteger) arg2).max((BigInteger) arg);
          } else if ("min".equalsIgnoreCase(func)) {
            Object arg2 = stack.removeLast();   // leftmost arg
            if (!(arg instanceof BigInteger) || !(arg2 instanceof BigInteger))
              throw new IllegalStateException("ExpressionParser.eval() both args not BigInteger min(" + arg2 + ' ' + arg + ')');
            arg = ((BigInteger) arg2).min((BigInteger) arg);
          } else if ("min".equalsIgnoreCase(func)) {
            Object arg2 = stack.removeLast();   // leftmost arg
            if (!(arg instanceof BigInteger) || !(arg2 instanceof BigInteger))
              throw new IllegalStateException("ExpressionParser.eval() both args not BigInteger min(" + arg2 + ' ' + arg + ')');
            arg = ((BigInteger) arg2).min((BigInteger) arg);
          } else if ("high".equalsIgnoreCase(func)) {
            if (!(arg instanceof BigInteger))
              throw new IllegalStateException("ExpressionParser.eval() arg not BigInteger high(" + arg + ')');
            arg = ((BigInteger) arg).divide(INT256).and(INT255);
          } else if ("low".equalsIgnoreCase(func)) {
            if (!(arg instanceof BigInteger))
              throw new IllegalStateException("ExpressionParser.eval() arg not BigInteger low(" + arg + ')');
            arg = ((BigInteger) arg).and(INT255);
          } else {
            throw new IllegalStateException("ExpressionParser.eval() unknown function '$" + func + '\'');
          }
          stack.add(arg == null ? NULL : arg);
          break;
        }
      }
      if (stack.size() > 1) {
        throw new IllegalStateException("Parser.eval() leftover on stack after eval");
      }
      return stack.removeLast();
    } catch (Exception ex) {
      IllegalStateException nex;
      if (expr.length > 0 && expr[0].type == Token.EXP) {
        nex = new IllegalStateException("Error evaluating: '" + expr[0].val + "'");
      } else {
        nex = new IllegalStateException("Error evaluating: " + tokensToString(expr));
      }
      nex.initCause(ex);
      throw nex;
    }
  }

  private static boolean evalBoolean (PrintStream out, String expr, Map vals, boolean expected) {
    boolean result = (Boolean) eval(parse(expr), vals);
    boolean err = result != expected;
    if (err)
      out.println(expr + " = " + result + ", expected " + expected);
    return err;
  }

  private static boolean evalBoolean (PrintStream out, String expr, boolean expected) {
    return evalBoolean(out, expr, new HashMap(), expected);
  }

  private static boolean evalInteger (PrintStream out, String expr, Map vals, BigInteger expected) {
    BigInteger result = (BigInteger) eval(parse(expr), vals);
    boolean err = !(result.compareTo(expected) == 0);
    if (err)
      out.println(expr + " = " + result + ", expected " + expected);
    return err;
  }

  private static boolean doTests (PrintStream out) {
    boolean err;
    err = evalInteger(out, "0xD8", new HashMap(), new BigInteger("D8", 16));
    // Test shortcut operators
    Map<String,Object> vals = new HashMap<>();
    vals.put("V1", "1");
    vals.put("V2", "2");
    vals.put("V3", null);
    err |= evalBoolean(out, "QQ == '1'", vals, false);  // Note: QQ undefined
    err |= evalBoolean(out, "QQ != '1'", vals, true);   // Note: QQ undefined
    err |= evalBoolean(out, "V3 != null  &&  V3 == 'TEST'", vals, false);
    err |= evalBoolean(out, "V3 == null  ||  V3 == 'TEST'", vals, true);
    err |= evalBoolean(out, "V2 == null  ||  V2 == 'TEST'", vals, false);
    err |= evalBoolean(out, "11 < 12", true);
    err |= evalBoolean(out, "11 < 11", false);
    err |= evalBoolean(out, "12 > 10", true);
    err |= evalBoolean(out, "11 > 11", false);
    err |= evalBoolean(out, "12 <= 12", true);
    err |= evalBoolean(out, "12 <= 12", true);
    err |= evalBoolean(out, "13 <= 12", false);
    err |= evalBoolean(out, "12 >= 12", true);
    err |= evalBoolean(out, "13 >= 12", true);
    err |= evalBoolean(out, "12 >= 13", false);
    err |= evalBoolean(out, "10 == 10", true);
    err |= evalBoolean(out, "10 == 11", false);
    err |= evalBoolean(out, "10 != 10", false);
    err |= evalBoolean(out, "10 != 11", true);
    err |= evalBoolean(out, "-2 < -1", true);
    err |= evalBoolean(out, "true & true", true);
    err |= evalBoolean(out, "true & false", false);
    err |= evalBoolean(out, "true | false", true);
    err |= evalBoolean(out, "!true ^ !false", true);
    err |= evalBoolean(out, "'XX' == 'XX'", true);
    err |= evalBoolean(out, "'XX' != 'YY'", true);
    err |= evalBoolean(out, "('XX' == 'XX') & true", true);
    vals.clear();
    vals.put("A", null);
    err |= evalBoolean(out, "(2 + 2) > (1 + 1)", true);
    err |= evalInteger(out, "(2 + 2) * (1 + 1)", new HashMap(), new BigInteger("8"));
    err |= evalInteger(out, "4 + 2", new HashMap(), new BigInteger("6"));
    err |= evalInteger(out, "4 - 2", new HashMap(), new BigInteger("2"));
    err |= evalInteger(out, "4 / 2", new HashMap(), new BigInteger("2"));
    err |= evalInteger(out, "(2 * (3 + 3)) / 2", new HashMap(), new BigInteger("6"));
    err |= evalInteger(out, "(1 ^ (1 | 2)) & 3", new HashMap(), new BigInteger("2"));
    err |= evalInteger(out, "(1 ^ !3) & 3", new HashMap(), new BigInteger("1"));
    err |= evalInteger(out, "5 % 2", new HashMap(), new BigInteger("1"));
    err |= evalInteger(out, "1 << 2", new HashMap(), new BigInteger("4"));
    err |= evalInteger(out, "-1 << 2", new HashMap(), new BigInteger("-4"));
    err |= evalInteger(out, "8 >> 2", new HashMap(), new BigInteger("2"));
    err |= evalInteger(out, "-8 >> 2", new HashMap(), new BigInteger("-2"));
    err |= evalInteger(out, "-8 >>> 2", new HashMap(), new BigInteger("-2"));
    vals.clear();
    vals.put("A", new BigInteger("10"));
    vals.put("B", new BigInteger("20"));
    err |= evalInteger(out, "A+B", vals, new BigInteger("30"));
    err |= evalInteger(out, "max(A B)", vals, new BigInteger("20"));
    err |= evalInteger(out, "min(A B)", vals, new BigInteger("10"));
    err |= evalInteger(out, "high(0x1234)", new HashMap(), new BigInteger("12", 16));
    err |= evalInteger(out, "low(0x1234)", new HashMap(), new BigInteger("34", 16));
    return err;
  }

  public static void main (String[] args) {
    boolean err = doTests(System.out);
    if (!err) {
      System.out.println("All tests pass!");
    }
  }
}
