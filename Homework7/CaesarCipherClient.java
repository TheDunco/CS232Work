import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.util.Scanner;

public class CaesarCipherClient {
    public static void main (String[] args) {
        if (args.length == 2) {
            CaesarCipherClient me = new CaesarCipherClient(args[0], Integer.parseInt(args[1]));
        }
        else {
            System.out.println("Usage: java CaesarCipherClient <host> <port>");
            System.exit(0);
        }

    }

    public CaesarCipherClient(String host, int port) {
        try {
            // Initialize socket
            Socket socket = new Socket(host, port);

            // Initialize in and out to read from and write to the socket
            PrintStream out = new PrintStream( socket.getOutputStream() );
            BufferedReader in = new BufferedReader( new InputStreamReader( socket.getInputStream() ));

            Scanner input = new Scanner(System.in);

            System.out.print("Welcome to the CaesarCipherClient! \nEnter your rotation number: ");

            int rotationNumber = input.nextInt();

            System.out.println("Thanks! Attempting to contact server...");

            out.print(rotationNumber);
            int response = in.read();

            System.out.println("Got response: " + response);

            // Close everything that's open to avoid leaks
            input.close();
            in.close();
            out.close();
            socket.close();

        } 
        catch( Exception e ) {
            e.printStackTrace();
        }
    }
}
