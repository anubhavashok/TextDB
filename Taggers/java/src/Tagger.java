import com.sun.net.httpserver.HttpServer;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * Created by anubhav on 11/17/15.
 */
public class Tagger {
    Status status;
    long ctr = 0;
    Boolean do_started_ack(HashMap<String, Object> args)
    {
        status = Status.STARTED;
        return true;
    }

    Boolean do_training_ack(HashMap<String, Object> args)
    {
        status = Status.TRAINING;
        return true;
    }

    Boolean do_active_ack(HashMap<String, Object> args)
    {
        status = Status.ACTIVE;
        return true;
    }

    Boolean do_disabled_ack(HashMap<String, Object> args)
    {
        status = Status.DISABLED;
        return true;
    }

    Object compute(HashMap<String, Object> args)
    {
        if (status != Status.ACTIVE) {
            return (Boolean)false;
        }
        return (Boolean)false;
    }

    Object train(HashMap<String, Object> args)
    {
        return true;
    }

    protected ArrayList<String> paths;
    public Object cmds(String cmd, HashMap<String, Object> args)
    {
        if (cmd.toLowerCase().equals("started_ack")) {
            return do_started_ack(args);
        } else if (cmd.toLowerCase().equals("training_ack")) {
            return do_training_ack(args);
        } else if (cmd.toLowerCase().equals("active_ack")) {
            return do_active_ack(args);
        } else if (cmd.toLowerCase().equals("disabled_ack")) {
            return do_disabled_ack(args);
        } else {
            return "Unknown";
        }
    }

    public String handle(String cmd, HashMap<String, Object> args)
    {
        return "";
    }
    public Tagger(int port) {
        this.port = port;
        paths = new ArrayList<String>() {
            {
                add("started_ack");
                add("training_ack");
                add("active_ack");
                add("disabled_ack");
            }
        };
    }

    public enum Status
    {
        DISABLED,
        STARTED,
        TRAINING,
        ACTIVE
    }
    int port = 8000;
    public void run() throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        for (String path: paths) {
            System.out.println("Adding path: " + path);
            server.createContext("/"+path, new RequestHandler(path, this));
        }
        server.setExecutor(null);
        server.start();
    }

    public long tick()
    {
        long c = ctr;
        ++ctr;
        return c;
    }

}
