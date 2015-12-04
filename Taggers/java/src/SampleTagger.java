import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by anubhav on 11/17/15.
 */
public class SampleTagger extends Tagger {

    public SampleTagger(int port)
    {
        super(port);
        // Add endpoints here
        paths.add("test");
    }
    @Override
    public Object cmds(String cmd, HashMap<String, Object> args)
    {
        if (cmd.toLowerCase().equals("test")) {
            return true;
        } else if (cmd.toLowerCase().equals("test2")) {
            return true;
        } else {
            // This part has to be there since you want to enable the basic message passing functionality
            return super.cmds(cmd, args);
        }
    }

    @Override
    public String handle(String cmd, HashMap<String, Object> args)
    {
        return cmds(cmd, args).toString();
    }
}
