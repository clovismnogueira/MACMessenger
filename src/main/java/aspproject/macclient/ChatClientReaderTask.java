package aspproject.macclient;

import android.os.AsyncTask;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.net.SocketTimeoutException;

/**
 *  AsyncTask class to run the socket reader in the background with no interference with the main
 *  UI Thread.
 *
 *
 *  @author Clóvis
 */

public class ChatClientReaderTask extends AsyncTask<Void, String, Void> {

    int port;
    String serverAddress;
    TextView txtLog;
    Socket clientSocket;

    /**
     *
     * @param serverAddress
     * @param port
     * @param txtLog
     * @throws IOException
     */
    ChatClientReaderTask (String serverAddress, int port, TextView txtLog) throws IOException {
        this.txtLog = txtLog;
        this.serverAddress = serverAddress;
        this.port = port;
        this.clientSocket = new Socket(this.serverAddress,this.port);
    }

    /**
     *
     *   Returns a reference to the Socket opened with the server.
     *
     * @return
     */
    public Socket getClientSocket() {
        return clientSocket;
    }

    /**
     *  Runs in the background as a different Thread as a Working Thread. This is not to interfere
     *  with the UI Thread, otherwise we might encounter Exceptions related to Thread management.
     *
     * @param parameter
     * @return
     */
    @Override
    protected Void doInBackground(Void... parameter) {
        try {
//            publishProgress("Socket connection open to the Server!");
            BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
            while(true) {
                // Reads from server socket
                String msgFromServer = reader.readLine();
                // Call the publish method. This will invoke, within the UI thread the method onProgressUpdate
                // This way, whatever is read from the InputStream from the socket will be managed by the
                // OnProgressUpdate method.
                publishProgress(msgFromServer);
            }
        } catch(SocketTimeoutException e) {
            e.printStackTrace();
        } catch(IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
     *  Updates the txtLog TextView with the messages received from the server socket
     *
     * @param message
     */
    @Override
    protected void onProgressUpdate(String... message) {
        super.onProgressUpdate(message);
        // This is will actually run in the UI Thread and therefore there is no issues to update
        // any of the UI components.
        String actualLog = txtLog.getText().toString();
        txtLog.setText(message[0] + "\n" + actualLog);
    }

}