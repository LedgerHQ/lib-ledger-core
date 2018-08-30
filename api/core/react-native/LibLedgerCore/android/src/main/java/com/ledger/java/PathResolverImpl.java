package com.ledger.java;

import android.content.Context;

import com.facebook.react.bridge.ReactApplicationContext;

import java.io.File;

/**
 * Module used to resolve file paths. libledger-core has its own iternal representation of the file system that may not
 * be adapted for the runtime platform. All path given to the PathResolver are absolute.
 */
public class PathResolverImpl extends co.ledger.core.PathResolver {
    private ReactApplicationContext reactContext;
    public PathResolverImpl(ReactApplicationContext reactContext) {
        this.reactContext = reactContext;
    }
    /**
     * Resolves the path for a SQLite database file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolveDatabasePath(String path) {
        String base = "database_";
		File file = this.reactContext.getFilesDir();
        String modifiedPath = file.getAbsolutePath().replace("/","__");
        return modifiedPath.concat(base);
    }

    /**
     * Resolves the path of a single log file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolveLogFilePath(String path) {
        String base = "log_file_";
        File file = this.reactContext.getFilesDir();
        String modifiedPath = file.getAbsolutePath().replace("/","__");
        return modifiedPath.concat(base);
    }

    /**
     * Resolves the path for a json file.
     * @param path The path to resolve.
     * @return The resolved path.
     */
    public String resolvePreferencesPath(String path) {
        String base = "preferences_";
        File file = this.reactContext.getFilesDir();
        String modifiedPath = file.getAbsolutePath().replace("/","__");
        return modifiedPath.concat(base);
    }
}
