import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.InputMismatchException;
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

            System.out.println("Host: " + host + "\nPort: " + port);
            // Initialize socket
            Socket socket = new Socket(host, port);

            // Initialize in and out to read from and write to the socket
            PrintStream out = new PrintStream( socket.getOutputStream() );
            BufferedReader in = new BufferedReader( new InputStreamReader( socket.getInputStream() ));

            // Initialize a scanner to grab input from the user
            Scanner input = new Scanner(System.in);

            System.out.print("Welcome to the CaesarCipherClient! \nEnter your rotation number: ");

            int rotationNumber = 0;

            // Grab the rotation number from the user
            try {
                rotationNumber = input.nextInt();

            }
            catch (InputMismatchException ime) {
                System.out.println("Invalid input: please enter an integer");

                input.close();
                in.close();
                out.close();
                socket.close();

                throw ime;
            }

            System.out.println("Thanks! Rotation number entered: " + rotationNumber);

            // Attempt to contact the server with that rotation number
            out.print(rotationNumber);
            System.out.println("Attempting to contact server...");

            // Wait for the server's response and respond appropriately
            int response = 0;
            try { 
                response = Integer.parseInt(in.readLine());
            }
            catch(java.io.IOException e) {
                System.out.println(e);
            }

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
