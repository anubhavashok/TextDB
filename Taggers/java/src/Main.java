/**
 * Created by anubhav on 11/18/15.
 */
public class Main {
    public static void main(String[] args) throws Exception {
        int port = Integer.valueOf(args[0]);
        Tagger t = new SampleTagger(port);
        t.run();
    }
}
