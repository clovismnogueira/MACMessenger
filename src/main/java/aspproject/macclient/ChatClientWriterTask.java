package aspproject.macclient;

import android.os.AsyncTask;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.net.SocketTimeoutException;

/**
 *   This is Class is incomplete and not being used.
  *
 * @author Clóvis
 */

public class ChatClientWriterTask extends AsyncTask<String, Void, String> {

    TextView txtLog;
    TextView txtMsg;
    Socket clientSocket;
    BufferedWriter writer;
    String clientMessage;

    ChatClientWriterTask (Socket socket, TextView log, TextView msg) throws IOException {
        this.txtLog = log;
        this.txtMsg = msg;
        this.clientSocket = socket;
    }

    /**
     *
     * @return
     */
    public Socket getClientSocket() {
        return clientSocket;
    }

    /**
     *
     * @param parameter
     * @return
     */
    @Override
    protected String doInBackground(String... clientMessage) {
//        try {
//            if (writer == null && clientSocket != null) {
//                this.writer = new BufferedWriter(new OutputStreamWriter(clientSocket.getOutputStream()));
//            } else if (writer != null) {
//                writer.write(clientMessage[0] + "\n");
//                writer.flush();
//            }
//        } catch(SocketTimeoutException e) {
//            e.printStackTrace();
//        } catch(IOException e) {
//            e.printStackTrace();
//        }
//        this.clientMessage = clientMessage[0] + "\n";

        System.out.println("I am in the doInBackGround of the Writer Thread msg :" + clientMessage[0]);
        return clientMessage[0];
    }

    @Override
    protected void onPostExecute(String response) {
//        String actualLog = txtLog.getText().toString();
//        String actualMsg = response;
//        txtLog.setText("#From Me: " + actualMsg + "\n" + actualLog);
//        txtMsg.setText("");
//        System.out.println("#From Me: " + actualMsg + "\n" + actualLog);
    }
}