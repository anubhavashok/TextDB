/**
 * Created by anubhav on 11/18/15.
 */

import org.apache.commons.cli.*;

public class Main {
    public static void main(String[] args) throws Exception {

        Options options = new Options();
        options.addOption("port", true, "Port in localhost that the current tagger is supposed to run on.");
        CommandLineParser parser = new DefaultParser();
        CommandLine cmd = parser.parse( options, args);

        if (cmd.hasOption("port")) {
            int port = Integer.parseInt(cmd.getOptionValue("port"));
            Tagger t = new SampleTagger(port);
            t.run();
        } else {
            HelpFormatter formatter = new HelpFormatter();
            formatter.printHelp("SampleTagger", options );
        }
    }
}
