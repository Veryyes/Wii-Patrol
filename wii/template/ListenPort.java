import java.net.Socket;
import java.net.ServerSocket;
import java.io.BufferedReader;

import java.io.IOException;
import java.io.InputStreamReader;

class ListenPort{

	public static void main(String[] args) throws IOException{
		int port = Integer.parseInt(args[0]);
		ServerSocket server = new ServerSocket(port);
		System.out.println("Waiting for Wii to connect");
		Socket socket = server.accept();
		BufferedReader br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		/*String str;
		while((str=br.readLine())!=null)
		{
			System.out.println(str);
		}*/
		java.util.Scanner scan = new java.util.Scanner(System.in);
		String input;
		System.out.print("Insert File Name:\t");
		while(!(input=scan.next()).equals("DONE")){
			System.out.println("Being Drawing!");
			java.io.FileWriter fw = new java.io.FileWriter(new java.io.File(input));
			int i;
			while((i=br.read())!=-1){
				fw.write((char)(i));
				if(i=='\n')
					break;
			}
			fw.close();
			System.out.println(input+" saved!");
			System.out.print("Insert File Name:\t");
		}
//		System.out.print(br.readLine());
		br.close();
	//	fw.close();
		socket.close();
		server.close();
	}
}
