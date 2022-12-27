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
		//System.out.println("HELLO WORLD");
		
		File file = new File("Map_Data.txt");
		
		Scanner scan = null;
		
		//tries to open the file
		try {scan = new Scanner(file);}
		catch (Exception e){
			System.out.println("File not found");
			System.exit(0);
		}
		
		String line;
		int x = 0;
		int y = 0;
		int val = 0;
		
		//checks if there is a next line
		for (int i = 0; scan.hasNextLine(); i++)
		{
			
		line = scan.nextLine();
		System.out.println(line);
		
		//sets x of point
		x = Integer.valueOf(line.substring(line.indexOf("(") + 1, line.indexOf(",")));
		//System.out.println(x);
		
		//set y of point
		y = Integer.valueOf(line.substring(line.indexOf(",") + 1, line.indexOf(")")));
		//System.out.println(y);
		
		//sets val of point
		val = Integer.valueOf(line.substring(line.indexOf("=") + 2, line.indexOf("=") + 3));
		//System.out.println(val);
		
		//set location x,y to val 
		map[x][y] = val;
		}
		
		
		//prints out map
		for (int i = 0; i < map.length; i++)
		{
			for (int j = 0; j < map.length; j++)
			{
				System.out.print(map[i][j]);
			}
			System.out.println("");
		}
		
		//creates frame to display 
		MappingJFrame frame = new MappingJFrame(map);


	}

}
