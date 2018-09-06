package com.ledger.java;

import co.ledger.core.HttpRequest;
import co.ledger.core.Error;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.HashMap;
import java.util.Map;

/**Class representing the http client performing the http requests */
public class HttpClientImpl extends co.ledger.core.HttpClient {
    /**
     *Execute a giver Http request\
     *@param request, HttpRequest object, requestr to execute
     */
    public void execute(co.ledger.core.HttpRequest request) {
        try {
            URL url = new URL(request.getUrl());

            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            HashMap<String, String> headers = request.getHeaders();
            for (String hr : headers.keySet()) {
                connection.setRequestProperty(hr, headers.get(hr));
            }

            int httpCode = connection.getResponseCode();

            BufferedInputStream iStream = new BufferedInputStream(connection.getInputStream());
            String response = getString(iStream, "UTF-8");

            HttpUrlConnectionImpl urlConnection = new HttpUrlConnectionImpl(response, httpCode, headers, null);
        } catch (IOException ex) {
            Error error = new Error(co.ledger.core.ErrorCode.HTTP_ERROR, ex.getMessage());
            HttpUrlConnectionImpl urlConnection = new HttpUrlConnectionImpl(ex.toString(), ex.hashCode(), null, error);
        }
    }

    private static String getString(InputStream stream, String charsetName) throws IOException
    {
        int n = 0;
        char[] buffer = new char[1024 * 4];
        InputStreamReader reader = new InputStreamReader(stream, charsetName);
        StringWriter writer = new StringWriter();
        while (-1 != (n = reader.read(buffer))) {
            writer.write(buffer, 0, n);
        }
        return writer.toString();
    }
}
