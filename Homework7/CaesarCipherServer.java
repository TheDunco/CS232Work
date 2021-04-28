import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Date;

/*
Based heavily on this example...
https://www.infoworld.com/article/2853780/socket-programming-for-scalable-systems.html
*/


public class CaesarCipherServer extends Thread {
    private ServerSocket serverSocket;
    private int port;
    private boolean running = false;

    public CaesarCipherServer(int port) {
        this.port = port;
    }

    public void startServer() {
        try {
            serverSocket = new ServerSocket(port);
            this.start();
        } 
        catch (IOException ioe) {
            ioe.printStackTrace();
        }
    }

    public void stopServer() {
        running = false;
        this.interrupt();
    }

    @Override
    public void run() {

        // Continually wait for new connections and spawn a request handler thread for every new connection
        running = true;
        while( running )
        {
            try
            {
                System.out.println( "Listening for a connection" );

                // Call accept() to receive the next connection
                Socket socket = serverSocket.accept();

                // Pass the socket to the RequestHandler thread for processing
                RequestHandler requestHandler = new RequestHandler( socket );
                requestHandler.start();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
    }

    public static void main( String[] args )
    {
        if( args.length == 0 )
        {
            System.out.println( "Usage: SimpleSocketServer <port>" );
            System.exit( 0 );
        }
        int port = Integer.parseInt( args[ 0 ] );
        System.out.println( "Start server on port: " + port );

        CaesarCipherServer server = new CaesarCipherServer( port );
        server.startServer();

        // Automatically shutdown in 1 minute
        try
        {
            Thread.sleep( 60000 );
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }

        server.stopServer();
    }
    
}

class RequestHandler extends Thread
{
    private Socket socket;
    RequestHandler( Socket socket )
    {
        this.socket = socket;
    }

    @Override
    public void run()
    {
        try
        {
            // Thanks to this tutorial for this date code...
            // https://www.edureka.co/blog/date-format-in-java/#:~:text=Creating%20A%20Simple%20Date%20Format,-A%20SimpleDateFormat%20is&text=String%20pattern%20%3D%20%22yyyy%2DMM,for%20formatting%20and%20parsing%20dates.
            Date date = new Date();
            SimpleDateFormat simpDate = new SimpleDateFormat("dd/MM hh:mm::ss");
            String stringDate = simpDate.format(date);

            System.out.println( "Received a connection " + stringDate);

            // Get input and output streams
            BufferedReader in = new BufferedReader( new InputStreamReader( socket.getInputStream() ) );
            PrintWriter out = new PrintWriter( socket.getOutputStream() );

            String rotationNumberString = null;
            int rotationInt = 0;

            // Make sure we get a number and follow the protocol for establishing a correct rotation number
            while(rotationNumberString == null) {
                try {
                    rotationNumberString = in.readLine();
                    rotationInt = Integer.parseInt(rotationNumberString);

                    // Make sure rotation number is positive
                    if (rotationInt <= 0 || rotationInt >= 25) {
                        out.println("Please enter a rotation number in the range 1..25");
                        out.flush();

                        rotationNumberString = null;
                        rotationInt = 0;

                        continue;
                    }
                }
                catch (Exception e) {
                    out.println("Silly, that's not valid a number... Try again");
                    out.flush();

                    rotationNumberString = null;
                }
            }

            // Echo back the valid rotationNumber we inevitably recieved
            out.println(rotationNumberString);
            out.flush();

            String stringToCipher = "";
            String cipheredString = "";

            // continue ciphering any further messages
            try {
                while(socket.isConnected())
                {
                    stringToCipher = in.readLine();

                    cipheredString = CaesarCipher(stringToCipher, rotationInt);

                    out.println(cipheredString);
                    out.flush();
                }
            }
            catch(NullPointerException e) { }

            // Close the connection
            in.close();
            out.close();
            socket.close();

            System.out.println( "CaesarCipherServer connection establsihed " + stringDate + " has been closed" );
        }
        catch( Exception e )
        {
            e.printStackTrace();
        }
    }

    // This method based off of the encrypt function of this example...
    // https://examples.javacodegeeks.com/caesar-cipher-java-example/#:~:text=It%20is%20a%20type%20of,to%20communicate%20with%20his%20generals.
    private String CaesarCipher(String stringToCipher, int rotationNumber) {
        StringBuffer result = new StringBuffer();

        for (int i = 0; i < stringToCipher.length(); i++) {
            
            // uppercase character, map to uppercase characters
            if (Character.isUpperCase(stringToCipher.charAt(i))) {
                char ch = (char) (((int) stringToCipher.charAt(i) +
                        rotationNumber - 65) % 26 + 65);
                result.append(ch);

            // lowercase character, map to lowercase characters
            } else if (Character.isLowerCase(stringToCipher.charAt(i))) {
                char ch = (char) (((int) stringToCipher.charAt(i) +
                        rotationNumber - 97) % 26 + 97);
                result.append(ch);

            // other symbol, don't touch it
            } else {
                result.append(stringToCipher.charAt(i));
            }
        }
        return result.toString();
    }
}