package org.nopcode.pancake.radare.swing;

import java.awt.*;
import javax.swing.*;

public class Main
{
  public static void main(String[] args)
  {
	JFrame frame = new JFrame("jradare");
	Container c = frame.getContentPane();
	JPanel panel = new JPanel();
	panel.setLayout(new FlowLayout());
	c.add(panel);
	panel.add(new JLabel("Hello"));
	panel.add(new JLabel("World"));
	frame.pack();
	frame.setVisible(true);
  }
}
