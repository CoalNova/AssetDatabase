import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.util.ArrayList;

public class Requestor
{
    public static void main(String[] args)
    {
        try
        {
            var socket = new Socket("127.0.0.1", 9864);
            var outStream = new PrintStream( socket.getOutputStream() );
            var inStream = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            //requestorID is sent plain, 
            //returned twiddled based on session hash, 
            //twiddled again based on server token, and sent with data
            //for now it's a simple bitwise op
            int requestorID = 12345678;

            var requestable = GeneratePrologue(requestorID);
            
            byte[] bytes = new byte[requestable.size()];
            for (int i = 0; i < bytes.length; i++) 
                bytes[i] = (byte)(requestable.get(i).charValue());
            outStream.write(bytes);


            //impl handshake
            char[] handshake = new char[4]; 
            inStream.read(handshake);
            
            System.out.println("sent" + requestorID + ", received " + ((handshake[0] << 24) + (handshake[1] << 16) + (handshake[2] << 8) + (handshake[3])));
            
            var supplies = new ArrayList<Supplies>();
            supplies.add(new Supplies(841815, 10));
            
            requestorID = (handshake[0] << 24) + (handshake[1] << 16) + (handshake[2] << 8) + (handshake[3]);
            System.out.println(requestorID + ", " + ((requestorID >> 1) | requestorID));
            requestorID = (requestorID >> 1) | requestorID;
            
            requestable = AggregateRequisition(supplies, requestorID);
            bytes = new byte[requestable.size()];
            for (int i = 0; i < bytes.length; i++) 
                bytes[i] = (byte)(requestable.get(i).charValue());
            
            outStream.write(bytes);
            

            inStream.close();
            outStream.close();
            socket.close();
        }
        catch (Exception e)
        {
            System.out.println("Sad failure");
            e.printStackTrace();
        }
    }

    public static ArrayList<Character> GeneratePrologue(int requestorID)
    {
        var list = new ArrayList<Character>();
        //session id or auth token?
        //byte 0 - 
        //  messenger header 101, 
        //  type 0-1 (request, intake)
        //  messenger footer 0101
        //  reqcount
        // hhht 
        list.add((char)0b01100101);
        
        list.add((char)((requestorID >> 24) & 255));
        list.add((char)((requestorID >> 16) & 255));
        list.add((char)((requestorID >> 8) & 255));
        list.add((char)((requestorID) & 255));

        return list;
    }

    public static ArrayList<Character> AggregateRequisition(ArrayList<Supplies> supplies, int requestorID)
    {
        var list = new ArrayList<Character>();
        
        list.add((char)((requestorID >> 24) & 255));
        list.add((char)((requestorID >> 16) & 255));
        list.add((char)((requestorID >> 8) & 255));
        list.add((char)((requestorID) & 255));
        //next is request length, hard capped at 64
        int requestSize = (supplies.size() < 64) ? supplies.size() : 63;
        list.add((char)requestSize);
        for (var item : supplies) {
            list.add((char)((item.itmCode >> 16) & 255));
            list.add((char)((item.itmCode >> 8) & 255));
            list.add((char)(item.itmCode & 255));
            list.add((char)(item.quantity & 255));
        }

        list.add('h');
        list.add('u');
        list.add('l');
        list.add('l');
        list.add('o');


        return list;
    }
}
