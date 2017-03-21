package aspproject.macclient;


import android.os.AsyncTask;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.UnknownHostException;

/**
 * Created by Clóvis on 19/03/2017.
 */

public class AsyncClient extends AsyncTask<Void, Void, Void> {

    String serverAddress;
    int port;
    TextView txtLog;
    String messageFromServer;
    Socket socket = null;

    AsyncClient(String address, int port, TextView txtLog) {
        this.serverAddress = address;
        this.port = port;
        this.txtLog = txtLog;
    }

    @Override
    protected Void doInBackground(Void... arg0) {
       try {
            socket = new Socket(this.serverAddress, this.port);
            BufferedReader reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            while(true) {
                messageFromServer = reader.readLine();
                System.out.println("Message from Server: " + messageFromServer);
            }
        } catch (UnknownHostException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        } finally {
            if (socket != null) {
                try {
                    socket.close();
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        txtLog.setText(messageFromServer);
        super.onPostExecute(result);
    }


}
