import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

import com.google.protobuf.CodedOutputStream;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.HashMap;

import tagger_message.TaggerMessage;

/**
 * Created by anubhav on 11/19/15.
 */
class RequestHandler implements HttpHandler {

    /**
     * returns the url parameters in a map
     * @param query
     * @return map
     */
    public static HashMap<String, Object> queryToMap(String query){
        HashMap<String, Object> result = new HashMap<String, Object>();
        if (query == null) {
            return result;
        }
        for (String param : query.split("&")) {
            String pair[] = param.split("=");
            if (pair.length>1) {
                result.put(pair[0], pair[1]);
            }else{
                result.put(pair[0], "");
            }
        }
        return result;
    }

    String cmd;
    Tagger tagger;
    RequestHandler(String _cmd, Tagger _tagger)
    {
        tagger = _tagger;
        cmd = _cmd;
    }
    public void handle(HttpExchange t) throws IOException {
        t.getResponseHeaders().set("Content-Type", "application/octet-stream");
        OutputStream os = t.getResponseBody();
        TaggerMessage.TagResult tagResult;
        try {
            HashMap<String, Object> args = queryToMap(t.getRequestURI().getQuery());
            TaggerMessage.TagRequest req = TaggerMessage.TagRequest.newBuilder().setCmd("").build();
            System.out.println("Handling cmd: " + cmd);
            System.out.println("cmd number: " + tagger.ctr);
            String response = tagger.handle(cmd, args);
            // contains appropriate tag/ result
            long val = 0;
            tagResult = TaggerMessage.TagResult.newBuilder()
                    .setDocumentName(req.getDocumentName())
                    .setResultID(tagger.tick())
                    .setSuccess(true)
                    .setTagResult(val)
                    .build();
            //os.write(response.getBytes());

        } catch (Exception e)
        {
            System.out.println("Exception: " + e.getMessage());
            tagResult = TaggerMessage.TagResult.newBuilder()
                    .setDocumentName("")
                    .setResultID(tagger.tick())
                    .setSuccess(false)
                    .build();
        }
        System.out.println("Response: " + tagResult.getSerializedSize());
        t.sendResponseHeaders(200, 0);
        tagResult.writeTo(os);
        os.flush();
        os.close();
    }
}
