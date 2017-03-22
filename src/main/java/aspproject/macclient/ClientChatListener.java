package aspproject.macclient;

import android.widget.TextView;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.SocketTimeoutException;

/**
 * Created by Clóvis on 19/03/2017.
 */

public class ClientChatListener extends Thread {

    TextView txtLog;
    String messageFromServer;
    Socket socket;


    public ClientChatListener(Socket socket, TextView txtLog) throws IOException {
        this.txtLog = txtLog;
        this.socket = socket;
    }

    @Override
    public void run() {
        try {
            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            while(true) {
                String msgFromServer = reader.readLine();
                String actualLog = txtLog.getText().toString();
                txtLog.setText(actualLog + msgFromServer + "\n");
            }
        } catch(SocketTimeoutException e) {
            e.printStackTrace();
        } catch(IOException e) {
            e.printStackTrace();
        }
    }
}