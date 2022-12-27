package Mapping;

import javax.swing.JFrame;

import java.awt.Graphics;
import java.awt.Color;

public class MappingJFrame extends JFrame
{
	
	int map[][] = new int[20][20];
	boolean drawAll = true;
	boolean drawRed = true;
	boolean drawBlue = false;
	boolean drawGreen = true;
	boolean drawYellow = true;
	
public MappingJFrame(int[][] mapIn)
{
 //Set JFrame title
 super("MappingJFrame");
 
 map = mapIn;

 //Set default close operation for JFrame
 setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

 //Set JFrame size
 setSize(800,800);

 //Make JFrame visible 
 setVisible(true);
}



public void paint(Graphics g)
{
 super.paint(g);

 int maxPix = map.length;
 
 for (int i = 0; i < maxPix; i++)
 {
	 for (int j = 0; j < maxPix; j++)
	 {
		 //System.out.println(j);
		 g.setColor(Color.BLACK);
		 g.drawRect(50 + i * 35,70 + j * 35,30,30);
		 
		 if (map[i][j] == 0)
		 {
			 g.setColor(Color.WHITE);
			 g.fillRect(50 + i * 35,70 + j * 35,30,30);
		 }
		 else if ((drawRed || drawAll) && map[i][j] == 1)
		 {
			 g.setColor(Color.RED); //N1 E2 S3 W4
			 g.fillRect(50 + i * 35,70 + j * 35,30,30);
		 }
		 else if ((drawGreen || drawAll) && map[i][j] == 2)
		 {
			 g.setColor(Color.GREEN);
			 g.fillRect(50 + i * 35,70 + j * 35,30,30);
		 }
		 else if ((drawBlue || drawAll) && map[i][j] == 3)
		 {
			 g.setColor(Color.BLUE);
			 g.fillRect(50 + i * 35,70 + j * 35,30,30);
		 }
		 else if ((drawYellow || drawAll) && map[i][j] == 4)
		 {
			 g.setColor(Color.YELLOW);
			 g.fillRect(50 + i * 35,70 + j * 35,30,30);
		 }
		 
		 //g.fillRect(50 + i * 35,70 + j * 35,30,30);
	 }
 }
 
}

}