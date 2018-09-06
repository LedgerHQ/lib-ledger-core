package com.ledger.java;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.net.URL;
import java.nio.file.DirectoryStream;
import java.nio.file.FileSystem;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.Collections;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Created by elkhalilbellakrid on 30/08/2018.
 */

public class NativeLibLoader {

    public static final String bindingNativeLibsJarPath = "binding/android/libs";

    public static final String bindingNativeLibsSysProp =
            "binding.native_libs_dirs";
    
    private static final Logger log =
            Logger.getLogger(NativeLibLoader.class.getName());

    private NativeLibLoader() { }

    // Load native libs from canonical locations
    public static void loadLibs() throws URISyntaxException, IOException {
        System.out.println("Loading");
        // Try to load from Jar
        loadLibsFromJarPath(bindingNativeLibsJarPath);

        // Try to load from system
        String localPaths = System.getProperty(bindingNativeLibsSysProp);
        if (localPaths != null) {
            log.log(Level.FINE, "Loading local native libs");
            for (String localPath : localPaths.split(",")) {
                loadLibsFromLocalPath(Paths.get(localPath));
            }
        }
    }

    // Load native lib(s) from the given `localPath` - a file or directory
    public static void loadLibsFromLocalPath(Path localPath) throws IOException {
        File localFile = localPath.toFile();
        if (!localFile.exists()) { return; }
        if (localFile.isDirectory()) {
            log.log(Level.FINE, " ######## Loading all libs in " + localFile.getAbsolutePath());
            for (File f : localFile.listFiles()) {
                if (f.isFile()) {
                    loadLibrary(f.getAbsolutePath());
                }
            }
        } else {
            loadLibrary(localFile.getAbsolutePath());
        }
    }

    
    public static void loadLibsFromJarPath(String jarPath)
            throws URISyntaxException, IOException {

        // Do we have a valid path?
        URL libsURL =
                NativeLibLoader.class
                        .getClassLoader()
                        .getResource(bindingNativeLibsJarPath);
        if (libsURL == null) { return; }

        // Are we actually referencing a jar path?
        if (!libsURL.toURI().getScheme().equals("jar")) { return; }

        log.log(Level.FINE, "####### Loading libs from jar path " + jarPath);

        // Walk the directory and load libs
        FileSystem fs =
                FileSystems.newFileSystem(libsURL.toURI(), Collections.<String, String>emptyMap());
        Path myPath = fs.getPath(jarPath);

        DirectoryStream<Path> directoryStream = Files.newDirectoryStream(myPath);
        try {
            for (Path p : directoryStream) {
                System.out.println(p.toString());
                loadLibFromJarPath(p);
            }
        } finally {
            directoryStream.close();
        }

        fs.close();
    }

    public static void loadLibFromJarPath(Path libPath) throws IOException {

        // System libraries *must* be loaded from the filesystem,
        // so we must copy the lib's data from the jar to a tempfile

        InputStream libIn =
                NativeLibLoader.class.getResourceAsStream(libPath.toString());
        if (libIn == null) { return; } // Invalid `libPath`

        // Name the tempfile
        String libName = libPath.getName(libPath.getNameCount() - 1).toString();
        // Name the tempfile after the lib to ease debugging
        String suffix = null;
        int extPos = libName.lastIndexOf('.');
        if (extPos > 0) { suffix = libName.substring(extPos + 1); }
        // Try to suffix the tempfile with the lib's suffix so that other
        // tools (e.g. profilers) identify the file correctly

        File tempLib = File.createTempFile(libName, suffix);
        tempLib.deleteOnExit();

        log.log(
                Level.FINE,
                "Copying jar lib " + libPath + " to " + tempLib.getAbsolutePath());
        try {
            Files.copy(libIn, tempLib.toPath(), StandardCopyOption.REPLACE_EXISTING);
        } catch (SecurityException e) {
            throw new RuntimeException(
                    "SecurityException while trying to create tempfile: " +
                            e.getMessage() + "\n\n If you cannot grant this process " +
                            "permissions to create temporary files, you need to install " +
                            "the native libraries manually and provide the installation " +
                            "path(s) using the system property " + bindingNativeLibsSysProp);
        }

        loadLibrary(tempLib.getAbsolutePath());
    }

    private static void loadLibrary(String abspath) {
        System.load(abspath);
        log.log(Level.INFO, "Loaded " + abspath);
    }
}
