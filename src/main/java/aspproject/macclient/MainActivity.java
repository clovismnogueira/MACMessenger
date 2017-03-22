package aspproject.macclient;

import android.os.StrictMode;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.InputType;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import java.io.BufferedWriter;
import java.io.OutputStreamWriter;
import java.net.Socket;

/**
 *
 *  Represents the main Activity of this Android application.
 *  This is a simple Socket Client called MAC Messenger to simulate
 *  communication within chatroom with professors in the MAC program.
 *
  *  @author Clóvis
 */
public class MainActivity extends AppCompatActivity {

    EditText txtIP;
    EditText txtPort;
    EditText txtLog;
    EditText txtMsg;
    Button btnConnect;
    Button btnSend;
    BufferedWriter writer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Keeping references of the Components in the Screen
        txtIP = (EditText) findViewById(R.id.txtIP);
        txtPort = (EditText) findViewById(R.id.txtPort);
        txtLog = (EditText) findViewById(R.id.txtLog);
        txtMsg = (EditText) findViewById(R.id.txtMsg);
        btnConnect = (Button) findViewById(R.id.btnConnect);
        btnSend = (Button) findViewById(R.id.btnSend);
        this.btnSend.setEnabled(false);
        // Sets the Log Text to:
        // this prevents the soft keyboard from being displayed. It also prevents pasting, allows
        // scrolling and doesn't alter the visual aspect of the view.
        // However, this also prevents selecting and copying of the text within the view.
//        this.txtLog.setInputType(InputType.TYPE_NULL);
        this.txtLog.setFocusable(false);

        // Just for testing purpose, this is not the best way to do it.
        // The Thread to listen to the Input Stream from the socket is done in the proper way using AsyncTask
        // But the Output Stream is forcing to use the UI Thread to write to the socket
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
    }

    public void connectServer(View view) {
        // Just a status message to show in the Status Screen
        String message = "Will connect to " + txtIP.getText() + " on Port " + txtPort.getText() + "\n";
        txtLog.setText(message);

        try {
            // Creating the socket connection
            String serverAddress = txtIP.getText().toString();
            int port = Integer.parseInt(txtPort.getText().toString());

            // This creates the AsyncThread to run in the background and read the socket from the Server.
            ChatClientReaderTask chatClientReaderTask = new ChatClientReaderTask(serverAddress, port, txtLog);
            // Starts the AsyncTask
            chatClientReaderTask.execute();

            // Creates the OutputStreamWriter
            // This is not the best way to do it. It is only working because the ThreadPolicy was changed
            // at the OnCreate method.
            Socket clientSocket = chatClientReaderTask.getClientSocket();
            if (clientSocket != null) {
                this.writer = new BufferedWriter(new OutputStreamWriter(clientSocket.getOutputStream()));

                this.btnConnect.setEnabled(false);
                this.btnSend.setEnabled(true);
                this.txtIP.setEnabled(false);
                this.txtPort.setEnabled(false);
            } else {
                message = "Connections PROBLEM, was not able to open socket with server!";
                txtLog.setText(message + "\n" + txtLog.getText().toString());
            }
        } catch(Exception ex) {
            ex.printStackTrace();
            txtLog.setText("Exception caught = " + ex);
        }
    }

    /**
     *
     * @param view
     */
    public void sendMsgServer(View view) {
        try {
            // Writes into the OutputStream Socket to the server. Sends the text from the txtMsg TextView
            writer.write(txtMsg.getText().toString());
            // As it is a Buffered Writer it needs to be flushed
            writer.flush();
            //Shows the message as sent
            String actualLog = txtLog.getText().toString();
            txtLog.setText("#From Me: " + txtMsg.getText().toString() + "\n" + actualLog);
            txtMsg.setText("");
        } catch(Exception ex) {
            ex.printStackTrace();
            txtLog.setText("Exception caught = " + ex);
        }
    }
}