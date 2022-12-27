package Mapping;

import java.util.Scanner;

import javax.swing.*;

import java.awt.BorderLayout;
import java.awt.Component;
import java.io.File;
import java.awt.Graphics;

public class Main {

	static int map [][] = new int[20][20];
	
	
	
	
	public static void main(String[] args) {
		System.out.println("HELLO WORLD");
		
		File file = new File("Map_Data.txt");
		
		Scanner scan = null;
		
		try {scan = new Scanner(file);}
		catch (Exception e){
			System.exit(0);
		}
		
		String line;
		String temp;
		int x = 0;
		int y = 0;
		int val = 0;
		
		for (int i = 0; scan.hasNextLine(); i++)
		{
		line = scan.nextLine();
		System.out.println(line);
		
		x = Integer.valueOf(line.substring(line.indexOf("(") + 1, line.indexOf(",")));
		//System.out.println(x);
		
		y = Integer.valueOf(line.substring(line.indexOf(",") + 1, line.indexOf(")")));
		//System.out.println(y);
		
		val = Integer.valueOf(line.substring(line.indexOf("=") + 2, line.indexOf("=") + 3));
		//System.out.println(val);
		
		map[x][y] = val;
		}
		
		
		for (int i = 0; i < map.length; i++)
		{
			for (int j = 0; j < map.length; j++)
			{
				System.out.print(map[i][j]);
			}
			System.out.println("");
		}
		
		
		DrawRectangleInJFrame frame = new DrawRectangleInJFrame(map);


	}

}
