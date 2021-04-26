import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.util.InputMismatchException;
import java.util.Scanner;

/*
Based on these two examples...
https://www.infoworld.com/article/2853780/socket-programming-for-scalable-systems.html
https://docs.oracle.com/javase/tutorial/networking/sockets/readingWriting.html
*/

public class CaesarCipherClient {

    Socket socket;
    PrintStream socketWriter;
    BufferedReader socketReader;
    Scanner userInput;

    public static void main (String[] args) {
        if (args.length == 2) {
            CaesarCipherClient me = new CaesarCipherClient(args[0], Integer.parseInt(args[1]));
            me.run();
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
            this.socket = new Socket(host, port);

            // Initialize in and out to read from and write to the socket
            this.socketWriter = new PrintStream( socket.getOutputStream() );
            this.socketReader = new BufferedReader( new InputStreamReader( socket.getInputStream() ));

            // Initialize a scanner to grab input from the user
            this.userInput = new Scanner(System.in);
        } 
        catch( Exception e ) {
            e.printStackTrace();
        }
    }

    public void run() {
        try {
            System.out.print("Welcome to the CaesarCipherClient! \nEnter your rotation number: ");
    
            int rotationNumber = 0;
    
            // Grab the rotation number from the user
            try {
                rotationNumber = this.userInput.nextInt();
    
            }
            catch (InputMismatchException ime) {
                System.out.println("Invalid input: please enter an integer");
    
                this.closeAll();
    
                throw ime;
            }
    
            System.out.println("Thanks! Rotation number entered: " + rotationNumber);
    
            // Attempt to contact the server with that rotation number
            socketWriter.println(rotationNumber);
            System.out.println("Attempting to contact server...");
    
            // Wait for the server's response and respond appropriately
            int response = 0;
            try { 
                response = Integer.parseInt(socketReader.readLine());
            }
            catch(java.io.IOException e) {
                System.out.println(e);
            }
    
            System.out.println("Got response: " + response);
    
            this.closeAll();
        }
        catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Close all open sockets/readers/writers to avoid leaks
    private void closeAll() {
        try {
            this.userInput.close();
            this.socketReader.close();
            this.socketWriter.close();
            this.socket.close();
        }
        catch (Exception e) {
            System.out.println("Closing failed");
            e.printStackTrace();
        }
    }
}
